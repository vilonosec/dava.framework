/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Particles/ParticleEmitter.h"
#include "Particles/Particle.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleLayer3D.h"
#include "Particles/ParticleLayerLong.h"
#include "Render/RenderManager.h"
#include "Utils/Random.h"
#include "Utils/StringFormat.h"
#include "Animation/LinearAnimation.h"
#include "Scene3D/Scene.h"
#include "FileSystem/FileSystem.h"
#include "Scene3D/SceneFileV2.h"

namespace DAVA 
{

REGISTER_CLASS(ParticleEmitter);

ParticleEmitter::ParticleEmitter()
{
	type = TYPE_PARTICLE_EMTITTER;
	Cleanup(false);

	bbox = AABBox3(Vector3(), Vector3());
}

ParticleEmitter::~ParticleEmitter()
{
	CleanupLayers();
}

void ParticleEmitter::Cleanup(bool needCleanupLayers)
{
	emitterType = EMITTER_POINT;
	emissionVector.Set(NULL);
	emissionVector = RefPtr<PropertyLineValue<Vector3> >(new PropertyLineValue<Vector3>(Vector3(1.0f, 0.0f, 0.0f)));
	emissionAngle.Set(NULL);
	emissionAngle = RefPtr<PropertyLineValue<float32> >(new PropertyLineValue<float32>(0.0f));
	emissionRange.Set(NULL);
	emissionRange = RefPtr<PropertyLineValue<float32> >(new PropertyLineValue<float32>(360.0f));
	size = RefPtr<PropertyLineValue<Vector3> >(0);
	colorOverLife = 0;
	radius = 0;
	rotationMatrix.Identity();
	// number = new PropertyLineValue<float>(1.0f);

	time = 0.0f;
	repeatCount = 0;
	lifeTime = 1000000000.0f;
	emitPointsCount = -1;
	isPaused = false;
	angle = 0.f;
	isAutorestart = true;
	particlesFollow = false;
    is3D = false;

	// Also cleanup layers, if needed.
	if (needCleanupLayers)
	{
		CleanupLayers();
	}
}

void ParticleEmitter::CleanupLayers()
{
    while(!layers.empty())
    {
        RemoveLayer(layers[0]);
    }
}

//ParticleEmitter * ParticleEmitter::Clone()
//{
//	ParticleEmitter * emitter = new ParticleEmitter();
//	for (int32 k = 0; k < (int32)layers.size(); ++k)
//	{
//		ParticleLayer * newLayer = layers[k]->Clone();
//		newLayer->SetEmitter(emitter);
//		emitter->layers.push_back(newLayer);
//	}
//    emitter->emissionVector = emissionVector;
//	if (emissionAngle)
//		emitter->emissionAngle = emissionAngle->Clone();
//	if (emissionRange)
//		emitter->emissionRange = emissionRange->Clone();
//	if(colorOverLife)
//		emitter->colorOverLife = colorOverLife->Clone();
//	if (radius)
//		emitter->radius = radius->Clone();
//    if (size)
//        emitter->size = size->Clone();
//	
//	emitter->type = type;
//	emitter->lifeTime = lifeTime;
//	emitter->emitPointsCount = emitPointsCount;
//	emitter->isPaused = isPaused;
//	emitter->isAutorestart = isAutorestart;
//	emitter->particlesFollow = particlesFollow;
//	return emitter;
//}

RenderObject * ParticleEmitter::Clone(RenderObject *newObject)
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<ParticleEmitter>(this), "Can clone only ParticleEmitter");
		newObject = new ParticleEmitter();
	}

	((ParticleEmitter*)newObject)->LoadFromYaml(configPath);

	return newObject;
}

void ParticleEmitter::Save(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	RenderObject::Save(archive, sceneFile);

	if(NULL != archive)
	{
		String filename = FileSystem::Instance()->AbsoluteToRelativePath(FileSystem::Instance()->GetCanonicalPath(sceneFile->GetScenePath()), FileSystem::Instance()->GetCanonicalPath(configPath));
		archive->SetString("pe.configpath", filename);
	}
}

void ParticleEmitter::Load(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	RenderObject::Load(archive, sceneFile);

	if(NULL != archive)
	{
		if(archive->IsKeyExists("pe.configpath"))
		{
            String filename = archive->GetString("pe.configpath");
            String sceneFilePath = FileSystem::Instance()->SystemPathForFrameworkPath(sceneFile->GetScenePath());
            configPath = FileSystem::Instance()->GetCanonicalPath(sceneFilePath + filename);

#if defined (__DAVAENGINE_ANDROID__)
            String systemPath = FileSystem::Instance()->SystemPathForFrameworkPath("~res:/");
            String::size_type pos = configPath.find(systemPath);
            if(pos == 0)
            {
                configPath = configPath.replace(pos, systemPath.length(), "~res:/");
            }
#endif //#if defined (__DAVAENGINE_ANDROID__)
            
			LoadFromYaml(configPath);
		}
	}
}

void ParticleEmitter::AddLayer(ParticleLayer * layer)
{
	if (layer)
	{
		layers.push_back(layer);
		layer->Retain();
		layer->SetEmitter(this);
		AddRenderBatch(layer->GetRenderBatch());
	}	
}

void ParticleEmitter::AddLayer(ParticleLayer * layer, ParticleLayer * layerToMoveAbove)
{
	AddLayer(layer);
	if (layerToMoveAbove)
	{
		MoveLayer(layer, layerToMoveAbove);
	}
}
	
void ParticleEmitter::RemoveLayer(ParticleLayer * layer)
{
	if (!layer)
	{
		return;
	}

	Vector<DAVA::ParticleLayer*>::iterator layerIter = std::find(layers.begin(), layers.end(), layer);
	if (layerIter != this->layers.end())
	{
		layers.erase(layerIter);

        RemoveRenderBatch(layer->GetRenderBatch());
		layer->SetEmitter(NULL);
		SafeRelease(layer);
	}
}
    
void ParticleEmitter::RemoveLayer(int32 index)
{
    DVASSERT(0 <= index && index < (int32)layers.size());

    RemoveLayer(layers[index]);
}

	
void ParticleEmitter::MoveLayer(ParticleLayer * layer, ParticleLayer * layerToMoveAbove)
{
	Vector<DAVA::ParticleLayer*>::iterator layerIter = std::find(layers.begin(), layers.end(), layer);
	Vector<DAVA::ParticleLayer*>::iterator layerToMoveAboveIter = std::find(layers.begin(), layers.end(),layerToMoveAbove);

	if (layerIter == layers.end() || layerToMoveAboveIter == layers.end() ||
		layerIter == layerToMoveAboveIter)
	{
		return;
	}
		
	layers.erase(layerIter);

	// Look for the position again - an iterator might be changed.
	layerToMoveAboveIter = std::find(layers.begin(), layers.end(),layerToMoveAbove);
	layers.insert(layerToMoveAboveIter, layer);
}

/* float32 ParticleEmitter::GetCurrentNumberScale()
{
	return number->GetValue(time);
} */

void ParticleEmitter::Play()
{
    Pause(false);
    DoRestart(false);
}
    
void ParticleEmitter::Stop()
{
    DoRestart(true);
    Pause(true);
}
    
bool ParticleEmitter::IsStopped()
{
    // Currently the same as isPaused.
    return isPaused;
}

void ParticleEmitter::Restart(bool isDeleteAllParticles)
{
	DoRestart(isDeleteAllParticles);
	Pause(false);
}
	
void ParticleEmitter::DoRestart(bool isDeleteAllParticles)
{
	Vector<ParticleLayer*>::iterator it;
	for(it = layers.begin(); it != layers.end(); ++it)
	{
		(*it)->Restart(isDeleteAllParticles);
	}

	time = 0.0f;
	repeatCount = 0;
}

void ParticleEmitter::Update(float32 timeElapsed)
{
	time += timeElapsed;
	float32 t = time / lifeTime;

	if (colorOverLife)
	{
		currentColor = colorOverLife->GetValue(t);
	}

	if(isAutorestart && (time > lifeTime))
	{
		time -= lifeTime;

        // Restart() resets repeatCount, so store it locally and then revert.
        int16 curRepeatCount = repeatCount;
		Restart(true);
        repeatCount = curRepeatCount;

		repeatCount ++;
	}

	Vector<ParticleLayer*>::iterator it;
	for(it = layers.begin(); it != layers.end(); ++it)
	{
        if(!(*it)->isDisabled)
            (*it)->Update(timeElapsed);
	}
}

void ParticleEmitter::RenderUpdate(Camera *camera, float32 timeElapsed)
{
	eBlendMode srcMode = RenderManager::Instance()->GetSrcBlend();
	eBlendMode destMode = RenderManager::Instance()->GetDestBlend();

	// Yuri Coder, 2013/01/30. ParticleEmitter class can be now only 2D.
	if(particlesFollow)
	{
		RenderManager::Instance()->PushDrawMatrix();
		RenderManager::Instance()->SetDrawTranslate(position);
	}

	Vector<ParticleLayer*>::iterator it;
	for(it = layers.begin(); it != layers.end(); ++it)
	{
		if(!(*it)->isDisabled)
			(*it)->Draw(camera);
	}

	if(particlesFollow)
	{
		RenderManager::Instance()->PopDrawMatrix();
	}

	RenderManager::Instance()->SetBlendMode(srcMode, destMode);
}

void ParticleEmitter::PrepareEmitterParameters(Particle * particle, float32 velocity, int32 emitIndex)
{
	// Yuri Coder, 2013/01/30. ParticleEmitter class can be now only 2D.
    Vector3 tempPosition = particlesFollow ? Vector3() : position;
    if (emitterType == EMITTER_POINT)
    {
        particle->position = tempPosition;
    }
    else if (emitterType == EMITTER_LINE)
    {
        // TODO: add emitter angle support
        float32 rand05 = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        Vector3 lineDirection(0, 0, 0);
        if(size)
            lineDirection = size->GetValue(time)*rand05;
        particle->position = tempPosition + lineDirection;
    }
    else if (emitterType == EMITTER_RECT)
    {
        // TODO: add emitter angle support
        float32 rand05_x = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        float32 rand05_y = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        float32 rand05_z = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        Vector3 lineDirection(0, 0, 0);
        if(size)
            lineDirection = Vector3(size->GetValue(time).x * rand05_x, size->GetValue(time).y * rand05_y, size->GetValue(time).z * rand05_z);
		particle->position = tempPosition + lineDirection;
	}
    else if (emitterType == EMITTER_ONCIRCLE)
    {
        // here just set particle position
        particle->position = tempPosition;
        //if (
    }
        
    Vector3 vel;
    //vel.x = (float32)((rand() & 255) - 128);
    //vel.y = (float32)((rand() & 255) - 128);
    //vel.Normalize();
        
    float32 rand05 = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]

    float32 particleAngle = 0;
    if(emissionAngle)
        particleAngle = DegToRad(emissionAngle->GetValue(time) + angle);
        
    float32 range = 0.0f;
    if(emissionRange)
        range = DegToRad(emissionRange->GetValue(time));
        
    if (emitPointsCount == -1)
    {
        // if emitAtPoints property is not set just emit randomly in range
        particleAngle += range * rand05;
    }
    else
    {
        particleAngle += range * (float32)emitIndex / (float32)emitPointsCount;
    }
        
        
    vel.x = cosf(particleAngle);
    vel.y = sinf(particleAngle);
    vel.z = 0;
        
    // reuse particle velocity we've calculated
    if (emitterType == EMITTER_ONCIRCLE)
    {
        if(radius)
            particle->position += vel * radius->GetValue(time);
    }
        
    particle->direction.x = vel.x;
    particle->direction.y = vel.y;
	particle->speed = velocity;
    particle->angle = particleAngle;
}

void ParticleEmitter::LoadFromYaml(const String & filename)
{
    Cleanup(true);
    
	YamlParser * parser = YamlParser::Create(filename);
	if(!parser)
	{
		Logger::Error("ParticleEmitter::LoadFromYaml failed (%s)", filename.c_str());
		return;
	}

	configPath = filename;
	time = 0.0f;
	repeatCount = 0;
	lifeTime = 1000000000.0f;

	YamlNode * rootNode = parser->GetRootNode();

	YamlNode * emitterNode = rootNode->Get("emitter");
	if (emitterNode)
	{
		if (emitterNode->Get("emissionAngle"))
			emissionAngle = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(emitterNode, "emissionAngle");
        
		if (emitterNode->Get("emissionVector"))
			emissionVector = PropertyLineYamlReader::CreateVector3PropertyLineFromYamlNode(emitterNode, "emissionVector");
        
		if (emitterNode->Get("emissionRange"))
			emissionRange = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(emitterNode, "emissionRange");
        
		if (emitterNode->Get("colorOverLife"))
			colorOverLife = PropertyLineYamlReader::CreateColorPropertyLineFromYamlNode(emitterNode, "colorOverLife");
		if (emitterNode->Get("radius"))
			radius = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(emitterNode, "radius");
		
		emitPointsCount = -1; 
		YamlNode * emitAtPointsNode = emitterNode->Get("emitAtPoints");
		if (emitAtPointsNode)
			emitPointsCount = emitAtPointsNode->AsInt();
		
		YamlNode * lifeTimeNode = emitterNode->Get("life");
		if (lifeTimeNode)
		{
			lifeTime = lifeTimeNode->AsFloat();
		}else
		{
			lifeTime = 1000000000.0f;
		}
        
        is3D = false;
		YamlNode * _3dNode = emitterNode->Get("3d");
		if (_3dNode)
		{	
			is3D = _3dNode->AsBool();
		}
        
		YamlNode * typeNode = emitterNode->Get("type");
		if (typeNode)
		{	
			if (typeNode->AsString() == "point")
				emitterType = EMITTER_POINT;
			else if (typeNode->AsString() == "line")
				emitterType = EMITTER_LINE;
			else if (typeNode->AsString() == "rect")
				emitterType = EMITTER_RECT;
			else if (typeNode->AsString() == "oncircle")
				emitterType = EMITTER_ONCIRCLE;
			else 
				emitterType = EMITTER_POINT;
		}else
			emitterType = EMITTER_POINT;
		
        size = PropertyLineYamlReader::CreateVector3PropertyLineFromYamlNode(emitterNode, "size");
        
        if(size == 0)
        {
            Vector3 _size(0, 0, 0);
            YamlNode * widthNode = emitterNode->Get("width");
            if (widthNode)
                _size.x = widthNode->AsFloat();

            YamlNode * heightNode = emitterNode->Get("height");
            if (heightNode)
                _size.y = heightNode->AsFloat();

            YamlNode * depthNode = emitterNode->Get("depth");
            if (depthNode)
                _size.y = depthNode->AsFloat();
            
            size = new PropertyLineValue<Vector3>(_size);
        }
        
		YamlNode * autorestartNode = emitterNode->Get("autorestart");
		if(autorestartNode)
			isAutorestart = autorestartNode->AsBool();

		YamlNode * particlesFollowNode = emitterNode->Get("particlesFollow");
		if(particlesFollowNode)
			particlesFollow = particlesFollowNode->AsBool();
	}

	int cnt = rootNode->GetCount();
	for (int k = 0; k < cnt; ++k)
	{
		YamlNode * node = rootNode->Get(k);
		YamlNode * typeNode = node->Get("type");
		
		YamlNode * longNode = node->Get("isLong");
		bool isLong = false;
		if(longNode && (longNode->AsBool() == true))
		{
			isLong = true;
		}

		if (typeNode && typeNode->AsString() == "layer")
		{
			LoadParticleLayerFromYaml(node, isLong);
		}
	}
	
	// Yuri Coder, 2013/01/15. The "name" node for Layer was just added and may not exist for
	// old yaml files. Generate the default name for nodes with empty names.
	UpdateEmptyLayerNames();
	
	SafeRelease(parser);
}

void ParticleEmitter::SaveToYaml(const String & filename)
{
    YamlParser* parser = YamlParser::Create();
    if (!parser)
    {
        Logger::Error("ParticleEmitter::SaveToYaml() - unable to create parser!");
        return;
    }
    
    YamlNode* rootYamlNode = new YamlNode(YamlNode::TYPE_MAP);
    YamlNode* emitterYamlNode = new YamlNode(YamlNode::TYPE_MAP);
    rootYamlNode->AddNodeToMap("emitter", emitterYamlNode);
    
    emitterYamlNode->Set("3d", this->is3D);
    emitterYamlNode->Set("type", GetEmitterTypeName());
    
    // Write the property lines.
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(emitterYamlNode, "emissionAngle", this->emissionAngle);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(emitterYamlNode, "emissionRange", this->emissionRange);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector3>(emitterYamlNode, "emissionVector", this->emissionVector);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(emitterYamlNode, "radius", this->radius);

    PropertyLineYamlWriter::WriteColorPropertyLineToYamlNode(emitterYamlNode, "colorOverLife", this->colorOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector3>(emitterYamlNode, "size", this->size);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(emitterYamlNode, "life", this->lifeTime);
    
    // Now write all the Layers. Note - layers are child of root node, not the emitter one.
    int32 layersCount = this->layers.size();
    for (int32 i = 0; i < layersCount; i ++)
    {
        this->layers[i]->SaveToYamlNode(rootYamlNode, i);
    }

    parser->SaveToYamlFile(filename, rootYamlNode, true);
    parser->Release();
}
    
int32 ParticleEmitter::GetParticleCount()
{
	int32 cnt = 0;
	Vector<ParticleLayer*>::iterator it;
	for(it = layers.begin(); it != layers.end(); ++it)
	{
		cnt += (*it)->GetParticleCount();
	}
	return cnt;
}

int32 ParticleEmitter::GetRepeatCount()
{
	return repeatCount;
}

int32 ParticleEmitter::GetEmitPointsCount()
{
	return emitPointsCount;
}
	
Vector<ParticleLayer*> & ParticleEmitter::GetLayers()
{
	return layers;
}
	
float32 ParticleEmitter::GetLifeTime()
{
	return lifeTime;
}
    
void ParticleEmitter::SetLifeTime(float32 time)
{
    lifeTime = time;
}
    
float32 ParticleEmitter::GetTime()
{
    return time;
}
    
void ParticleEmitter::Pause(bool _isPaused)
{
	isPaused = _isPaused;
}
bool ParticleEmitter::IsPaused()
{
	return isPaused;
}

void ParticleEmitter::SetAutorestart(bool _isAutorestart)
{
	isAutorestart = _isAutorestart;
}

bool ParticleEmitter::GetAutorestart()
{
	return isAutorestart;
}

Vector3 ParticleEmitter::GetSize()
{
    if(size)
        return size->GetValue(0);
    return Vector3(0, 0, 0);
}
    
Vector3 ParticleEmitter::GetSize(float32 time)
{
    if(size)
        return size->GetValue(time);
    return Vector3(0, 0, 0);
}
    
void ParticleEmitter::SetSize(const Vector3& _size)
{
	size = new PropertyLineValue<Vector3>(_size);
}

Animation * ParticleEmitter::SizeAnimation(const Vector3 & newSize, float32 time, Interpolation::FuncType interpolationFunc /*= Interpolation::LINEAR*/, int32 track /*= 0*/)
{
    Vector3 _size(0, 0, 0);
    if(size)
        _size = size->GetValue(0);
	LinearAnimation<Vector3> * animation = new LinearAnimation<Vector3>(this, &_size, newSize, time, interpolationFunc);
	animation->Start(track);
	return animation;
}

String ParticleEmitter::GetEmitterTypeName()
{
    switch (this->emitterType)
    {
        case EMITTER_POINT:
        {
            return "point";
        }

        case EMITTER_LINE:
        {
            return "line";
        }

        case EMITTER_RECT:
        {
            return "rect";
        }

        case EMITTER_ONCIRCLE:
        {
            return "oncircle";
        }

        default:
        {
            return "unknown";
        }
    }
}

void ParticleEmitter::UpdateEmptyLayerNames()
{
	int32 layersCount = this->GetLayers().size();
	for (int i = 0; i < layersCount; i ++)
	{
		UpdateLayerNameIfEmpty(this->layers[i], i);
	}
}

void ParticleEmitter::UpdateLayerNameIfEmpty(ParticleLayer* layer, int32 index)
{
	if (layer && layer->layerName.empty())
	{
		layer->layerName = Format("Layer %i", index);
	}
}

void ParticleEmitter::ReloadLayerSprites()
{
	int32 layersCount = this->GetLayers().size();
	for (int i = 0; i < layersCount; i ++)
	{
		this->GetLayers()[i]->ReloadSprite();
	}
}

void ParticleEmitter::LoadParticleLayerFromYaml(YamlNode* yamlNode, bool isLong)
{
	ParticleLayer* layer = new ParticleLayer();
	AddLayer(layer);
	layer->LoadFromYaml(configPath, yamlNode);

	SafeRelease(layer);
}

bool ParticleEmitter::Is3DFlagCorrect()
{
	// ParticleEmitter class can be created only for non-3D Emitters.
	return (is3D == false);
}

};
