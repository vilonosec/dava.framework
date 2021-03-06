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
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleSystem.h"
#include "Utils/StringFormat.h"
#include "Render/RenderManager.h"
#include "Render/Image.h"
#include "Utils/Random.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{

ParticleLayer::ParticleLayer()
	: head(0)
	, count(0)
	, limit(1000)
	, emitter(0)
	, sprite(0)
{
	renderBatch = new ParticleLayerBatch();
	renderBatch->SetParticleLayer(this);

	life = 0;
	lifeVariation = 0;

	number = 0;	
	numberVariation = 0;		

	size = 0;
	sizeVariation = 0;
	sizeOverLife = 0;

	velocity = 0;
	velocityVariation = 0;	
	velocityOverLife = 0;

	spin = 0;			
	spinVariation = 0;
	spinOverLife = 0;

	motionRandom = 0;		
	motionRandomVariation = 0;
	motionRandomOverLife = 0;

	bounce = 0;				
	bounceVariation = 0;
	bounceOverLife = 0;
	
	colorOverLife = 0;
	colorRandom = 0;
	alphaOverLife = 0;

	particlesToGenerate = 0.0f;
	alignToMotion = 0.0f;
	
	angle = 0;
	angleVariation = 0;

	layerTime = 0.0f;
	additive = true;
	type = TYPE_PARTICLES;
    
    endTime = 100000000.0f;

	frameStart = 0;
	frameEnd = 0;

	frameOverLifeEnabled = false;
	frameOverLifeFPS = 0;

    isDisabled = false;
}

ParticleLayer::~ParticleLayer()
{
	SafeRelease(renderBatch);
	SafeRelease(sprite);
	head = 0;
	CleanupForces();
	// dynamic cache automatically delete all particles
}

ParticleLayer * ParticleLayer::Clone(ParticleLayer * dstLayer)
{
	if(!dstLayer)
	{
		dstLayer = new ParticleLayer();
	}

	if (life)
		dstLayer->life.Set(life->Clone());
	
	if (lifeVariation)
		dstLayer->lifeVariation.Set(lifeVariation->Clone());

	if (number)
		dstLayer->number.Set(number->Clone());
	
	if (numberVariation)
		dstLayer->numberVariation.Set(numberVariation->Clone());

	if (size)
		dstLayer->size.Set(size->Clone());
	
	if (sizeVariation)
		dstLayer->sizeVariation.Set(sizeVariation->Clone());
	
	if (sizeOverLife)
		dstLayer->sizeOverLife.Set(sizeOverLife->Clone());
	
	if (velocity)
		dstLayer->velocity.Set(velocity->Clone());
	
	if (velocityVariation)
		dstLayer->velocityVariation.Set(velocityVariation->Clone());
	
	if (velocityOverLife)
		dstLayer->velocityOverLife.Set(velocityOverLife->Clone());

	// Copy the forces.
	for (int32 f = 0; f < (int32)forces.size(); ++ f)
	{
		ParticleForce* clonedForce = new ParticleForce(this->forces[f]);
		dstLayer->AddForce(clonedForce);
	}

	if (spin)
		dstLayer->spin.Set(spin->Clone());
	
	if (spinVariation)
		dstLayer->spinVariation.Set(spinVariation->Clone());
	
	if (spinOverLife)
		dstLayer->spinOverLife.Set(spinOverLife->Clone());
	
	if (motionRandom)
		dstLayer->motionRandom.Set(motionRandom->Clone());
	
	if (motionRandomVariation)
		dstLayer->motionRandomVariation.Set(motionRandomVariation->Clone());
	
	if (motionRandomOverLife)
		dstLayer->motionRandomOverLife.Set(motionRandomOverLife->Clone());
	
	if (bounce)
		dstLayer->bounce.Set(bounce->Clone());
	
	if (bounceVariation)
		dstLayer->bounceVariation.Set(bounceVariation->Clone());
	
	if (bounceOverLife)
		dstLayer->bounceOverLife.Set(bounceOverLife->Clone());
	
	if (colorOverLife)
		dstLayer->colorOverLife.Set(colorOverLife->Clone());
	
	if (colorRandom)
		dstLayer->colorRandom.Set(colorRandom->Clone());
	
	if (alphaOverLife)
		dstLayer->alphaOverLife.Set(alphaOverLife->Clone());
	
	if (angle)
		dstLayer->angle.Set(angle->Clone());
	
	if (angleVariation)
		dstLayer->angleVariation.Set(angleVariation->Clone());
	
	dstLayer->layerName = layerName;
	dstLayer->alignToMotion = alignToMotion;
	dstLayer->additive = additive;
	dstLayer->startTime = startTime;
	dstLayer->endTime = endTime;
	dstLayer->type = type;
	dstLayer->sprite = SafeRetain(sprite);
	dstLayer->pivotPoint = pivotPoint;
	
	dstLayer->frameStart = frameStart;
	dstLayer->frameEnd = frameEnd;

	dstLayer->frameOverLifeEnabled = frameOverLifeEnabled;
	dstLayer->frameOverLifeFPS = frameOverLifeFPS;

    dstLayer->isDisabled = isDisabled;
    
	return dstLayer;
}
	

void ParticleLayer::SetEmitter(ParticleEmitter * _emitter)
{
	emitter = _emitter;
}

void ParticleLayer::SetSprite(Sprite * _sprite)
{
    DeleteAllParticles();
	SafeRelease(sprite);
	sprite = SafeRetain(_sprite);
	UpdateFrameTimeline();

	if(sprite)
	{
		pivotPoint = Vector2(_sprite->GetWidth()/2.0f, _sprite->GetHeight()/2.0f);

		String spritePath = FileSystem::GetCanonicalPath(sprite->GetRelativePathname());
		const String configPath = FileSystem::Instance()->GetCanonicalPath(emitter->GetConfigPath());
		String configFolder, configFile;
		FileSystem::SplitPath(configPath, configFolder, configFile);
		relativeSpriteName = FileSystem::AbsoluteToRelativePath(configFolder, spritePath);
	}
}
	
Sprite * ParticleLayer::GetSprite()
{
	return sprite;
}

float32 ParticleLayer::GetLayerTime()
{
    return layerTime;
}
    
void ParticleLayer::DeleteAllParticles()
{
	if(TYPE_SINGLE_PARTICLE == type)
	{

	}
	Particle * current = head;
	while(current)
	{
		Particle * next = current->next;
		ParticleSystem::Instance()->DeleteParticle(current);
		count--;
		current = next;
	}
	head = 0;
	
	DVASSERT(count == 0);
}

void ParticleLayer::RunParticle(Particle * particle)
{
	particle->next = head;
	head = particle;
	count++;
}


/*void ParticleLayer::SetLimit(int32 _limit)
{
	limit = _limit;
}

void ParticleLayer::SetPosition(Vector3 _position)
{
	position = _position;
}*/

void ParticleLayer::Restart(bool isDeleteAllParticles)
{
	if (isDeleteAllParticles)
		DeleteAllParticles();

	layerTime = 0.0f;
	particlesToGenerate = 0.0f;
}


void ParticleLayer::Update(float32 timeElapsed)
{
	// increment timer	
	layerTime += timeElapsed;

	switch(type)
	{
	case TYPE_PARTICLES:
		{
			Particle * prev = 0;
			Particle * current = head;
			while(current)
			{
				Particle * next = current->next;
				if (!current->Update(timeElapsed))
				{
					if (prev == 0)head = next;
					else prev->next = next;
					ParticleSystem::Instance()->DeleteParticle(current);
					count--;
				}else
				{
					ProcessParticle(current);
					prev = current;
				}

				current = next;
			}
			
			if (count == 0)
			{
				DVASSERT(head == 0);
			}
			
			if ((layerTime >= startTime) && (layerTime < endTime) && !emitter->IsPaused())
			{
				float32 randCoeff = (float32)(Rand() & 255) / 255.0f;
				float32 newParticles = 0.0f;
				if (number)
					newParticles += timeElapsed * number->GetValue(layerTime);
				if (numberVariation)
					newParticles += randCoeff * timeElapsed * numberVariation->GetValue(layerTime);
				//newParticles *= emitter->GetCurrentNumberScale();
				particlesToGenerate += newParticles;

				while(particlesToGenerate >= 1.0f)
				{
					particlesToGenerate -= 1.0f;
					
					int32 emitPointsCount = emitter->GetEmitPointsCount();
					
					if (emitPointsCount == -1)
						GenerateNewParticle(-1);
					else {
						for (int k = 0; k < emitPointsCount; ++k)
							 GenerateNewParticle(k);
					}

				}
			}
			break;
		}
	case TYPE_SINGLE_PARTICLE:
		{
			bool needUpdate = true;
			if ((layerTime >= startTime) && (layerTime < endTime) && !emitter->IsPaused())
			{
				if(!head)
				{
					GenerateSingleParticle();
					needUpdate = false;
				}
			}
            if(head && needUpdate)
            {
				if (!head->Update(timeElapsed))
				{
					ParticleSystem::Instance()->DeleteParticle(head);
					count--;
					DVASSERT(0 == count);
					head = 0;
					if ((layerTime >= startTime) && (layerTime < endTime) && !emitter->IsPaused())
					{
						GenerateSingleParticle();
					}
				}
				else
				{
					ProcessParticle(head);
				}
            }
			
			break;		
		}
	}
}
	
void ParticleLayer::GenerateSingleParticle()
{
	GenerateNewParticle(-1);
	
	// Yuri Coder, 2013/03/26. head->angle = 0.0f commented out because of DF-877.
	//head->angle = 0.0f;

	//particle->velocity.x = 0.0f;
	//particle->velocity.y = 0.0f;
}

void ParticleLayer::GenerateNewParticle(int32 emitIndex)
{
	if (count >= limit)
	{
		return;
	}
	
	Particle * particle = ParticleSystem::Instance()->NewParticle();
	particle->CleanupForces();

	particle->next = 0;
	particle->sprite = sprite;
	particle->life = 0.0f;

	float32 randCoeff = (float32)(Rand() & 255) / 255.0f;
	
	
	particle->color = Color();
	if (colorRandom)
	{
		particle->color = colorRandom->GetValue(randCoeff);
	}
	

	particle->lifeTime = 0.0f;
	if (life)
		particle->lifeTime += life->GetValue(layerTime);
	if (lifeVariation)
		particle->lifeTime += (lifeVariation->GetValue(layerTime) * randCoeff);
	
	// size 
	particle->size = Vector2(0.0f, 0.0f); 
	if (size)
		particle->size = size->GetValue(layerTime);
	if (sizeVariation)
		particle->size +=(sizeVariation->GetValue(layerTime) * randCoeff);
	
	if(sprite)
	{
		particle->size.x /= (float32)sprite->GetWidth();
		particle->size.y /= (float32)sprite->GetHeight();
	}
	else
	{
		particle->size = Vector2(0, 0);
	}

	float32 vel = 0.0f;
	if (velocity)
		vel += velocity->GetValue(layerTime);
	if (velocityVariation)
		vel += (velocityVariation->GetValue(layerTime) * randCoeff);
	
	particle->angle = 0.0f;
	particle->spin = 0.0f;
	if (spin)
		particle->spin = DegToRad(spin->GetValue(layerTime));
	if (spinVariation)
		particle->spin += DegToRad(spinVariation->GetValue(layerTime) * randCoeff);

	//particle->position = emitter->GetPosition();	
	// parameters should be prepared inside prepare emitter parameters
	emitter->PrepareEmitterParameters(particle, vel, emitIndex);
	//particle->angle += alignToMotion;
	if (angle)
		particle->angle += DegToRad(angle->GetValue(layerTime));
	if (angleVariation)
		particle->angle += DegToRad(angleVariation->GetValue(layerTime) * randCoeff);

	particle->sizeOverLife = 1.0f;
	particle->velocityOverLife = 1.0f;
	particle->spinOverLife = 1.0f;
//	particle->forceOverLife0 = 1.0f;
//		
//	particle->force0.x = 0.0f;
//	particle->force0.y = 0.0f;
//
//	if ((forces.size() == 1) && (forces[0]))
//	{
//		particle->force0 = forces[0]->GetValue(layerTime);
//	}
//	if ((forcesVariation.size() == 1) && (forcesVariation[0]))
//	{
//		particle->force0 += forcesVariation[0]->GetValue(layerTime) * randCoeff;
//	}

	// Add the forces.
    int32 forcesCount = (int32)forces.size();
    for(int i = 0; i < forcesCount; i++)
	{
		Vector3 toAddForceDirection;
		float32 toAddForceValue = 0;
		float32 toAddForceOverlife = 0;
		bool toAddForceOvelifeEnabled = false;

		RefPtr<PropertyLine<Vector3> > currentForce = forces[i]->GetForce();
        if(currentForce && currentForce.Get())
		{
			const Vector3 & force = currentForce->GetValue(layerTime);
			float32 forceValue = force.Length();
			Vector3 forceDirection;
			if(forceValue)
			{
				forceDirection = force/forceValue;
			}

			toAddForceDirection = forceDirection;
			toAddForceValue = forceValue;
		}
		
		// Check the forces variations.
		RefPtr<PropertyLine<Vector3> > currentForceVariation = forces[i]->GetForceVariation();
        if(currentForceVariation && currentForceVariation.Get())
		{
			const Vector3 & force = currentForceVariation->GetValue(layerTime) * randCoeff;
			float32 forceValue = force.Length();
			Vector3 forceDirection = force/forceValue;

			toAddForceDirection += forceDirection;
			toAddForceValue += forceValue;
		}
		
		// Now check the overlife flag.
		RefPtr<PropertyLine<float32> > currentForceOverlife = forces[i]->GetForceOverlife();
		if(currentForceOverlife && currentForceOverlife.Get())
		{
			toAddForceOverlife = currentForceOverlife->GetValue(layerTime);
			toAddForceOvelifeEnabled = true;
		}
		
		// All the data is calculated - can add the force data to particle.
		particle->AddForce(toAddForceValue, toAddForceDirection, toAddForceOverlife, toAddForceOvelifeEnabled);
	}
    
	particle->frame = frameStart + (int32)(randCoeff * (float32)(frameEnd - frameStart));
	
	// process it to fill first life values
	ProcessParticle(particle);

	// go to life
	RunParticle(particle);
}

void ParticleLayer::ProcessParticle(Particle * particle)
{
	float32 t = particle->life / particle->lifeTime;
	if (sizeOverLife)
		particle->sizeOverLife = sizeOverLife->GetValue(t);
	if (spinOverLife)
		particle->spinOverLife = spinOverLife->GetValue(t);
	if (velocityOverLife)
		particle->velocityOverLife = velocityOverLife->GetValue(t);
	if (colorOverLife)
		particle->color = colorOverLife->GetValue(t);
	if (alphaOverLife)
		particle->color.a = alphaOverLife->GetValue(t);

	Color emitterColor;
	if(emitter->GetCurrentColor(&emitterColor))
	{
		particle->drawColor = particle->color*emitterColor*emitter->ambientColor;
	}
	else
	{
		particle->drawColor = particle->color*emitter->ambientColor;
	}

	// Frame Overlife FPS defines how many frames should be displayed in a second.
	// This property is cycled - if we reached the last frame, we'll update to the new one.
	if (frameOverLifeEnabled && frameOverLifeFPS > 0)
	{
		float32 timeElapsed = particle->life - particle->frameLastUpdateTime;
		if (timeElapsed > (1 / frameOverLifeFPS))
		{
			particle->frame ++;
			if (particle->frame >= this->sprite->GetFrameCount())
			{
				particle->frame = 0;
			}

			particle->frameLastUpdateTime = particle->life;
		}
	}
    
	int32 forcesCount = (int32)forces.size();
    for(int i = 0; i < forcesCount; i++)
	{
        if (forces[i]->GetForceOverlife() && forces[i]->GetForceOverlife().Get())
		{
			particle->UpdateForceOverlife(i, forces[i]->GetForceOverlife()->GetValue(t));
		}
	}
}

void ParticleLayer::Draw(Camera * camera)
{
	if (additive)
	{
		RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ONE);
	}
	else 
	{
		RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	}

	switch(type)
	{
		case TYPE_PARTICLES:
		{
			Particle * current = head;
			while(current)
			{
				sprite->SetPivotPoint(pivotPoint);
				current->Draw();
				current = current->next;
			}
			break;
		}
		case TYPE_SINGLE_PARTICLE:
		{
			if(head)
			{	
				sprite->SetPivotPoint(pivotPoint);
				head->Draw();
			}
			break;
		}
	}
}


void ParticleLayer::LoadFromYaml(const String & configPath, YamlNode * node)
{
// 	PropertyLine<float32> * life;				// in seconds
// 	PropertyLine<float32> * lifeVariation;		// variation part of life that added to particle life during generation of the particle
// 
// 	PropertyLine<float32> * number;				// number of particles per second
// 	PropertyLine<float32> * numberVariation;		// variation part of number that added to particle count during generation of the particle
// 
// 	PropertyLine<Vector2> * size;				// size of particles in pixels 
// 	PropertyLine<Vector2> * sizeVariation;		// size variation in pixels
// 	PropertyLine<float32> * sizeOverLife;
// 
// 	PropertyLine<float32> * velocity;			// velocity in pixels
// 	PropertyLine<float32> * velocityVariation;	
// 	PropertyLine<float32> * velocityOverLife;
// 
// 	PropertyLine<Vector2> * weight;				// weight property from 
// 	PropertyLine<Vector2> * weightVariation;
// 	PropertyLine<float32> * weightOverLife;
// 
// 	PropertyLine<float32> * spin;				// spin of angle / second
// 	PropertyLine<float32> * spinVariation;
// 	PropertyLine<float32> * spinOverLife;
// 
// 	PropertyLine<float32> * motionRandom;		//
// 	PropertyLine<float32> * motionRandomVariation;
// 	PropertyLine<float32> * motionRandomOverLife;
// 
// 	PropertyLine<float32> * bounce;				//
// 	PropertyLine<float32> * bounceVariation;
// 	PropertyLine<float32> * bounceOverLife;	
	
	
	type = TYPE_PARTICLES;
	YamlNode * typeNode = node->Get("layerType");
	if (typeNode)
	{
		if (typeNode->AsString() == "single")
			type = TYPE_SINGLE_PARTICLE;
	}

	YamlNode * nameNode = node->Get("name");
	if (nameNode)
	{
		layerName = nameNode->AsString();
	}
	
	YamlNode * spriteNode = node->Get("sprite");
	if (spriteNode)
	{
		YamlNode * pivotPointNode = node->Get("pivotPoint");
		
		const String relativePathName = spriteNode->AsString();
		relativeSpriteName = relativePathName;
		String configFolder, configFile;
		FileSystem::SplitPath(configPath, configFolder, configFile);
		
		String path = FileSystem::Instance()->GetCanonicalPath(configFolder+relativePathName);
		Sprite * _sprite = Sprite::Create(path);
		Vector2 pivotPointTemp;
		if(pivotPointNode)
		{
			pivotPointTemp = pivotPointNode->AsPoint();
		}
		SetSprite(_sprite);
		pivotPoint = Vector2(_sprite->GetWidth() / 2.0f + pivotPointTemp.x, _sprite->GetHeight() / 2.0f + pivotPointTemp.y);
        SafeRelease(_sprite);
	}


	colorOverLife = PropertyLineYamlReader::CreateColorPropertyLineFromYamlNode(node, "colorOverLife");
	colorRandom = PropertyLineYamlReader::CreateColorPropertyLineFromYamlNode(node, "colorRandom");
	alphaOverLife = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "alphaOverLife");
	
	YamlNode* frameOverLifeEnabledNode = node->Get("frameOverLifeEnabled");
	if (frameOverLifeEnabledNode)
	{
		frameOverLifeEnabled = frameOverLifeEnabledNode->AsBool();
	}

	YamlNode* frameOverLifeFPSNode = node->Get("frameOverLifeFPS");
	if (frameOverLifeFPSNode)
	{
		frameOverLifeFPS = frameOverLifeFPSNode->AsFloat();
	}

	life = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "life");	
	lifeVariation = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "lifeVariation");	

	number = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "number");	
	numberVariation = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "numberVariation");	

	size = PropertyLineYamlReader::CreateVector2PropertyLineFromYamlNode(node, "size");	
	sizeVariation = PropertyLineYamlReader::CreateVector2PropertyLineFromYamlNode(node, "sizeVariation");	
	sizeOverLife = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "sizeOverLife");	

	velocity = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "velocity");	
	velocityVariation = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "velocityVariation");	
	velocityOverLife = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "velocityOverLife");
	
	angle = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "angle");
	angleVariation = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "angleVariation");
	
	int32 forceCount = 0;
	YamlNode * forceCountNode = node->Get("forceCount");
	if (forceCountNode)
		forceCount = forceCountNode->AsInt();

	for (int k = 0; k < forceCount; ++k)
	{
        // Any of the Force Parameters might be NULL, and this is acceptable.
		RefPtr< PropertyLine<Vector3> > force = PropertyLineYamlReader::CreateVector3PropertyLineFromYamlNode(node, Format("force%d", k) );
		RefPtr< PropertyLine<Vector3> > forceVariation = PropertyLineYamlReader::CreateVector3PropertyLineFromYamlNode(node, Format("forceVariation%d", k));
		RefPtr< PropertyLine<float32> > forceOverLife = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, Format("forceOverLife%d", k));

		ParticleForce* particleForce = new ParticleForce(force, forceVariation, forceOverLife);
		AddForce(particleForce);
	}

	spin = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "spin");
	spinVariation = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "spinVariation");	
	spinOverLife = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "spinOverLife");	

	motionRandom = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "motionRandom");	
	motionRandomVariation = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "motionRandomVariation");	
	motionRandomOverLife = PropertyLineYamlReader::CreateFloatPropertyLineFromYamlNode(node, "motionRandomOverLife");	


	YamlNode * blend = node->Get("blend");
	if (blend)
	{
		if (blend->AsString() == "alpha")
			additive = false;
		if (blend->AsString() == "add")
			additive = true;
	}

	YamlNode * alignToMotionNode = node->Get("alignToMotion");
	if (alignToMotionNode)
		alignToMotion = DegToRad(alignToMotionNode->AsFloat());

	startTime = 0.0f;
	endTime = 100000000.0f;
	YamlNode * startTimeNode = node->Get("startTime");
	if (startTimeNode)
		startTime = startTimeNode->AsFloat();

	YamlNode * endTimeNode = node->Get("endTime");
	if (endTimeNode)
		endTime = endTimeNode->AsFloat();
	
	frameStart = 0;
	frameEnd = 0;

	YamlNode * frameNode = node->Get("frame");
	if (frameNode)
	{
		if (frameNode->GetType() == YamlNode::TYPE_STRING)
			frameStart = frameEnd = frameNode->AsInt();
		else if (frameNode->GetType() == YamlNode::TYPE_ARRAY)
		{
			frameStart = frameNode->Get(0)->AsInt();
			frameEnd = frameNode->Get(1)->AsInt();
		}
	}

	YamlNode * isDisabledNode = node->Get("isDisabled");
	if (isDisabledNode)
	{
		isDisabled = isDisabledNode->AsBool();
	}
	
	// Yuri Coder, 2013/01/31. After all the data is loaded, check the Frame Overlife timelines and
	// synchronize them with the maximum available frames in the sprite. See also DF-573.
	UpdateFrameTimeline();
}

void ParticleLayer::SaveToYamlNode(YamlNode* parentNode, int32 layerIndex)
{
    YamlNode* layerNode = new YamlNode(YamlNode::TYPE_MAP);
    String layerNodeName = Format("layer%d", layerIndex);
    parentNode->AddNodeToMap(layerNodeName, layerNode);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "name", layerName);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "type", "layer");
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "layerType",
                                                                 this->type == TYPE_SINGLE_PARTICLE ? "single" : "particles");
    if (this->IsLong())
    {
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "isLong", true);
    }
    
    if (this->sprite)
    {
        Vector2 pivotPoint(this->pivotPoint.x - (this->sprite->GetWidth() / 2.0f),
                           this->pivotPoint.y - (this->sprite->GetHeight() / 2.0f));
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<Vector2>(layerNode, "pivotPoint", pivotPoint);
    }

    // Truncate an extension of the sprite file.
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "sprite",
        this->relativeSpriteName.substr(0, this->relativeSpriteName.size()-4));

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "life", this->life);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "lifeVariation", this->lifeVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "number", this->number);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "numberVariation", this->numberVariation);
    
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector2>(layerNode, "size", this->size);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector2>(layerNode, "sizeVariation", this->sizeVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "sizeOverLife", this->sizeOverLife);
    
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "velocity", this->velocity);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "velocityVariation", this->velocityVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "velocityOverLife", this->velocityOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "spin", this->spin);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "spinVariation", this->spinVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "spinOverLife", this->spinOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "motionRandom", this->motionRandom);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "motionRandomVariation", this->motionRandomVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "motionRandomOverLife", this->motionRandomOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "bounce", this->bounce);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "bounceVariation", this->bounceVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "bounceOverLife", this->bounceOverLife);
	
	PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "angle", this->angle);
	PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "angleVariation", this->angleVariation);

    PropertyLineYamlWriter::WriteColorPropertyLineToYamlNode(layerNode, "colorRandom", this->colorRandom);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "alphaOverLife", this->alphaOverLife);

    PropertyLineYamlWriter::WriteColorPropertyLineToYamlNode(layerNode, "colorOverLife", this->colorOverLife);
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "frameOverLifeEnabled", this->frameOverLifeEnabled);
	PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "frameOverLifeFPS", this->frameOverLifeFPS);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "alignToMotion", this->alignToMotion);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "blend", this->additive ? "add" : "alpha");

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "startTime", this->startTime);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "endTime", this->endTime);

	PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "isDisabled", this->isDisabled);

    // Now write the forces.
    SaveForcesToYamlNode(layerNode);
}

void ParticleLayer::SaveForcesToYamlNode(YamlNode* layerNode)
{
    int32 forceCount = (int32)this->forces.size();
    if (forceCount == 0)
    {
        // No forces to write.
        return;
    }

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<int32>(layerNode, "forceCount", forceCount);
    for (int32 i = 0; i < forceCount; i ++)
    {
		ParticleForce* currentForce = this->forces[i];
        String forceDataName = Format("force%d", i);
		
        PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector3>(layerNode, forceDataName, currentForce->GetForce());

		// Force Variation and Force Overlife might be empty, so will not be stored in Yaml. This is handled
		// by PropertyLineYamlWriter, no further changes should be done.
        forceDataName = Format("forceVariation%d", i);
        PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector3>(layerNode, forceDataName, currentForce->GetForceVariation());

        forceDataName = Format("forceOverLife%d", i);
        PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, forceDataName, currentForce->GetForceOverlife());
    }
}

Particle * ParticleLayer::GetHeadParticle()
{
	return head;
}

const String & ParticleLayer::GetRelativeSpriteName()
{
	return relativeSpriteName;
}

RenderBatch * ParticleLayer::GetRenderBatch()
{
	return renderBatch;
}

void ParticleLayer::ReloadSprite()
{
	if (this->sprite)
	{
		this->sprite->Reload();
	}
	
	UpdateFrameTimeline();
}

void ParticleLayer::UpdateFrameTimeline()
{
	if (!this->frameOverLifeEnabled)
	{
		return;
	}

	if (!this->sprite)
	{
		this->frameOverLifeEnabled = false;
		return;
	}
}
	
void ParticleLayer::SetAdditive(bool additive)
{
	this->additive = additive;
}

void ParticleLayer::AddForce(ParticleForce* force)
{
	this->forces.push_back(force);
}

void ParticleLayer::UpdateForce(int32 forceIndex, RefPtr< PropertyLine<Vector3> > force,
										RefPtr< PropertyLine<Vector3> > forceVariation,
										RefPtr< PropertyLine<float32> > forceOverLife)
{
	if (forceIndex <= (int32)this->forces.size())
	{
		this->forces[forceIndex]->Update(force, forceVariation, forceOverLife);
	}
}

void ParticleLayer::RemoveForce(ParticleForce* force)
{
	Vector<ParticleForce*>::iterator iter = std::find(this->forces.begin(),
													  this->forces.end(),
													  force);
	if (iter != this->forces.end())
	{
		this->forces.erase(iter);
		SafeDelete(*iter);
	}
}

void ParticleLayer::RemoveForce(int32 forceIndex)
{
	if (forceIndex <= (int32)this->forces.size())
	{
		SafeDelete(this->forces[forceIndex]);
		this->forces.erase(this->forces.begin() + forceIndex);
	}
}

void ParticleLayer::CleanupForces()
{
	for (Vector<ParticleForce*>::iterator iter = this->forces.begin();
		 iter != this->forces.end(); iter ++)
	{
		SafeDelete(*iter);
	}
	
	this->forces.clear();
}

}
