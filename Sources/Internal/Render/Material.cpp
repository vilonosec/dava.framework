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
#include "Render/Material.h"
#include "Render/UberShader.h"
#include "Render/Texture.h"
#include "FileSystem/KeyedArchive.h"
#include "Utils/StringFormat.h"
#include "Render/Shader.h"
#include "Render/Image.h"
#include "Render/3D/PolygonGroup.h"
#include "Scene3D/DataNode.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "Render/Highlevel/Light.h"
#include "Render/TextureDescriptor.h"
#include "Platform/SystemTimer.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "FileSystem/FileSystem.h"

namespace DAVA 
{

REGISTER_CLASS(InstanceMaterialState)
    
InstanceMaterialState::InstanceMaterialState()
    :   flatColor(1.0f, 1.0f, 1.0f, 1.0f)
    ,   texture0Shift(0.0f, 0.0f)

{
    for (int32 k = 0; k < LIGHT_NODE_MAX_COUNT; ++k)
        lightNodes[k] = 0;
    lightmapTexture = 0;
}

InstanceMaterialState::~InstanceMaterialState()
{
    for (int32 k = 0; k < LIGHT_NODE_MAX_COUNT; ++k)
        SafeRelease(lightNodes[k]);
    SafeRelease(lightmapTexture);
}

void InstanceMaterialState::SetLight(int32 lightIndex, Light * lightNode)
{ 
    SafeRelease(lightNodes[lightIndex]);
    lightNodes[lightIndex] = SafeRetain(lightNode); 
}

Light * InstanceMaterialState::GetLight(int32 lightIndex) 
{ 
    return lightNodes[lightIndex]; 
}

void InstanceMaterialState::SetLightmap(Texture * _lightMapTexture, const String & _lightmapName)
{
    SafeRelease(lightmapTexture);
    lightmapTexture = SafeRetain(_lightMapTexture);
    lightmapName = _lightmapName;
}

void InstanceMaterialState::SetUVOffsetScale(const Vector2 & _uvOffset, const Vector2 _uvScale)
{
    uvOffset = _uvOffset;
    uvScale = _uvScale;
}

void InstanceMaterialState::ClearLightmap()
{
	SafeRelease(lightmapTexture);
	lightmapName = String("");
}

    
void InstanceMaterialState::SetFlatColor(const Color & color)
{
    flatColor = color;
}

const Color & InstanceMaterialState::GetFlatColor()
{
    return flatColor;
}


void InstanceMaterialState::SetTextureShift(const Vector2 & speed)
{
    texture0Shift = speed;
}

const Vector2 & InstanceMaterialState::GetTextureShift()
{
    return texture0Shift;
}

void InstanceMaterialState::Save(KeyedArchive * archive, SceneFileV2 *sceneFile)
{
	if(NULL != archive)
	{
		if(uvOffset != Vector2())
		{
			archive->SetVector2("ims.uvoffset", uvOffset);
		}
        
		if(uvScale != Vector2())
		{
			archive->SetVector2("ims.uvscale", uvScale);
		}
		
		if(!lightmapName.empty())
		{
            String filename = FileSystem::Instance()->AbsoluteToRelativePath(sceneFile->GetScenePath(), lightmapName);
            archive->SetString("ims.lightmapname", filename);
		}
		
		if(flatColor != Color::White())
		{
			archive->SetByteArrayAsType("ims.flatColor", flatColor);
		}
		
		if(texture0Shift != Vector2())
		{
			archive->SetVector2("ims.texture0Shift", texture0Shift);
		}
	}
}

void InstanceMaterialState::Load(KeyedArchive * archive, SceneFileV2 *sceneFile)
{
	if(NULL != archive)
	{
		uvOffset = archive->GetVector2("ims.uvoffset");
		uvScale = archive->GetVector2("ims.uvscale");

		String filename = archive->GetString("ims.lightmapname");
		if(!filename.empty())
		{
            String lName = FileSystem::Instance()->GetCanonicalPath(sceneFile->GetScenePath() + filename);

			Texture* lTexture = Texture::CreateFromFile(lName);
			SetLightmap(lTexture, lName);
			lTexture->Release();
		}
        else
        {
			SetLightmap(NULL, String(""));
        }

		flatColor = archive->GetByteArrayAsType("ims.flatColor", Color::White());
		texture0Shift = archive->GetVector2("ims.texture0Shift");
	}
}

InstanceMaterialState * InstanceMaterialState::Clone()
{
	InstanceMaterialState * newState = new InstanceMaterialState();

	newState->lightmapTexture = SafeRetain(lightmapTexture);
	newState->lightmapName = lightmapName;
	newState->uvOffset = uvOffset;
	newState->uvScale = uvScale;
	newState->flatColor = flatColor;
	newState->texture0Shift = texture0Shift;

	return newState;
}


REGISTER_CLASS(Material);
    
UberShader * Material::uberShader = 0;
    
const char8 * Material::GetTypeName(eType format)
{
    switch(format)
    {
        case MATERIAL_UNLIT_TEXTURE:
            return "UNLIT_TEXTURE";
        case MATERIAL_UNLIT_TEXTURE_DETAIL:
            return "UNLIT_TEXTURE_DETAIL";
        case MATERIAL_UNLIT_TEXTURE_DECAL:
            return "UNLIT_TEXTURE_DECAL";
        case MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
            return "UNLIT_TEXTURE_LIGHTMAP";
        case MATERIAL_VERTEX_LIT_TEXTURE:
            return "VERTEX_LIT_TEXTURE";
        case MATERIAL_VERTEX_LIT_DETAIL:
            return "VERTEX_LIT_DETAIL";
        case MATERIAL_VERTEX_LIT_DECAL:
            return "VERTEX_LIT_DECAL";
        case MATERIAL_VERTEX_LIT_LIGHTMAP:
            return "VERTEX_LIT_LIGHTMAP";
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
            return "PIXEL_LIT_NORMAL_DIFFUSE";
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
            return "PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR";
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
            return "PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP";
		case MATERIAL_VERTEX_COLOR_ALPHABLENDED:
			return "VERTEX_COLOR_ALPHABLEND";
		case MATERIAL_FLAT_COLOR:
			return "FLAT_COLOR";
        default:
            break;
    };
    return "WRONG MATERIAL";
}


Material::Material() 
    :   DataNode()
    ,   diffuseColor(0.8f, 0.8f, 0.8f, 1.0f)
    ,   specularColor(0.0f, 0.0f, 0.0f, 1.0f)
    ,   ambientColor(0.2f, 0.2f, 0.2f, 1.0f)
    ,   emissiveColor(0.0f, 0.0f, 0.0f, 1.0f)
    ,   shininess(1.0f)
    ,   isTranslucent(false)
    ,   isTwoSided(false)
	,	isSetupLightmap(false)
	,	setupLightmapSize(32)
    ,   isFogEnabled(false)
    ,   fogDensity(0.006f)
    ,   fogColor((float32)0x87 / 255.0f, (float32)0xbe / 255.0f, (float32)0xd7 / 255.0f, 1.0f)
	,	isAlphablend(false)
    ,   isFlatColorEnabled(false)
	,	blendSrc(BLEND_ONE)
	,	blendDst(BLEND_ONE)
	,	renderStateBlock()
    ,   isWireframe(false)
    ,   isTexture0ShiftEnabled(false)
    ,   isExportOwnerLayerEnabled(true)
    ,   ownerLayerName(LAYER_OPAQUE)
	,	viewOptions(MATERIAL_VIEW_TEXTURE_LIGHTMAP)
{
    //Reserve memory for Collection
    names.resize(TEXTURE_COUNT);
    
    
	renderStateBlock.state = RenderState::DEFAULT_3D_STATE;

//    if (scene)
//    {
//        DataNode * materialsNode = scene->GetMaterials();
//        materialsNode->AddNode(this);
//    }
    
    if (!uberShader)
    {
        uberShader = new UberShader();
        uberShader->LoadShader("~res:/Shaders/Default/materials.shader");
        
        
        //uberShader->CompileShaderCombination("MATERIAL_TEXTURE");
        //uberShader->CompileShaderCombination("MATERIAL_DECAL");
        //uberShader->CompileShaderCombination("MATERIAL_DETAIL");
        //
        //uberShader->CompileShaderCombination("MATERIAL_TEXTURE;VERTEX_LIT");
        //uberShader->CompileShaderCombination("MATERIAL_DECAL;VERTEX_LIT");
        //uberShader->CompileShaderCombination("MATERIAL_DETAIL;VERTEX_LIT");
        //
        //uberShader->CompileShaderCombination("MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE;");
        //uberShader->CompileShaderCombination("MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE;SPECULAR;");
        //uberShader->CompileShaderCombination("MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE;SPECULAR;GLOSS;");
    }
    
//    type = MATERIAL_UNLIT_TEXTURE;
//    shader = uberShader->GetShader("MATERIAL_TEXTURE");
    for (int32 tc = 0; tc < TEXTURE_COUNT; ++tc)
        textures[tc] = 0;

    SetType(MATERIAL_UNLIT_TEXTURE);
}
    
void Material::SetScene(Scene * _scene)
{
    DVASSERT(scene == 0);
    scene = _scene;
}


int32 Material::Release()
{
    int32 retainCount = BaseObject::Release();
    return retainCount;
}

Material::~Material()
{
    for (int32 tc = 0; tc < TEXTURE_COUNT; ++tc)
    {
        SafeRelease(textures[tc]);
    }
}
    
    
Material::eValidationResult Material::Validate(PolygonGroup * polygonGroup)
{
    RebuildShader();
    /*
        General check if number of attributes in shader is 
     */
    int32 format = polygonGroup->GetFormat();
    int32 formatBitCount = 0; // number of attributes in polygroup available in the shader
    for (int bit = 1; bit <= EVF_HIGHER_BIT; bit <<= 1)
    {
        if (format & bit)
        {
            if (shader->GetAttributeIndex((eVertexFormat)bit) != -1)
                formatBitCount++;
        }
    }
    // Check if we have more attributes 
    if (shader->GetAttributeCount() > formatBitCount)
    {
        //shader->Get
        
        
        
        return VALIDATE_INCOMPATIBLE;
    }
    
    
    return VALIDATE_COMPATIBLE;
}

void Material::RebuildShader()
{
    uniformTexture0 = -1;
    uniformTexture1 = -1;
    uniformLightPosition0 = -1;
    uniformMaterialLightAmbientColor = -1;
    uniformMaterialLightDiffuseColor = -1;
    uniformMaterialLightSpecularColor = -1;
    uniformMaterialSpecularShininess = -1;
    uniformLightIntensity0 = -1;
    uniformLightAttenuationQ = -1;
    uniformUvOffset = -1;
    uniformUvScale = -1;
    uniformFogDensity = -1;
    uniformFogColor = -1;
    uniformFlatColor = -1;
    uniformTexture0Shift = -1;
    
    String shaderCombileCombo = "";
    
    switch (type) 
    {
        case MATERIAL_UNLIT_TEXTURE:
            shaderCombileCombo = "MATERIAL_TEXTURE";
            break;
        case MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
			shaderCombileCombo = "MATERIAL_TEXTURE; MATERIAL_LIGHTMAP";
			if(isSetupLightmap)
			{
				shaderCombileCombo = shaderCombileCombo + ";SETUP_LIGHTMAP";
			}
			break;
        case MATERIAL_UNLIT_TEXTURE_DECAL:
            shaderCombileCombo = "MATERIAL_TEXTURE;MATERIAL_DECAL";
            break;
        case MATERIAL_UNLIT_TEXTURE_DETAIL:
            shaderCombileCombo = "MATERIAL_TEXTURE;MATERIAL_DETAIL";
            break;
        case MATERIAL_VERTEX_LIT_TEXTURE:
            shaderCombileCombo = "MATERIAL_TEXTURE;VERTEX_LIT";
            break;
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
            shaderCombileCombo = "MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE";
            break;
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
            shaderCombileCombo = "MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE;SPECULAR";
            break;
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
            shaderCombileCombo = "MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE;SPECULAR;GLOSS";
            break;
		case MATERIAL_VERTEX_COLOR_ALPHABLENDED:
			shaderCombileCombo = "MATERIAL_TEXTURE;ALPHABLEND;VERTEX_COLOR";
			break;
        case MATERIAL_FLAT_COLOR:
            isFlatColorEnabled = true;
            break;
        default:
            break;
    };

	switch (viewOptions)
	{
		case MATERIAL_VIEW_TEXTURE_LIGHTMAP:
			break;
		case MATERIAL_VIEW_LIGHTMAP_ONLY:
			if (shaderCombileCombo.size() > 0)shaderCombileCombo += ";";
			shaderCombileCombo = shaderCombileCombo + "MATERIAL_VIEW_LIGHTMAP_ONLY";
			break;
		case MATERIAL_VIEW_TEXTURE_ONLY:
			if (shaderCombileCombo.size() > 0)shaderCombileCombo += ";";
			shaderCombileCombo = shaderCombileCombo + "MATERIAL_VIEW_TEXTURE_ONLY";
			break;
		default:
			break;
	}

    if (isTranslucent)
    {
        if (shaderCombileCombo.size() > 0)shaderCombileCombo += ";";
        shaderCombileCombo = shaderCombileCombo + "OPAQUE";
    }

	if(isAlphablend)
	{
        if (shaderCombileCombo.size() > 0)shaderCombileCombo += ";";
		shaderCombileCombo = shaderCombileCombo + "ALPHABLEND";
	}
    
    if (isFlatColorEnabled)
    {
        if (shaderCombileCombo.size() > 0)shaderCombileCombo += ";";
        shaderCombileCombo = shaderCombileCombo + "FLATCOLOR";
    }
    
    if (isTexture0ShiftEnabled)
    {
        if (shaderCombileCombo.size() > 0)shaderCombileCombo += ";";
        shaderCombileCombo = shaderCombileCombo + "TEXTURE0_SHIFT_ENABLED";
    }
    
    
    //if (isDistanceAttenuation)
    //shaderCombileCombo = shaderCombileCombo + ";DISTANCE_ATTENUATION";
    
    if (isFogEnabled && RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
        shaderCombileCombo = shaderCombileCombo + ";VERTEX_FOG";

    // Get shader if combo unavailable compile it
    shader = uberShader->GetShader(shaderCombileCombo);
    
    switch (type) {
        case MATERIAL_UNLIT_TEXTURE:
            break;
        case MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
        case MATERIAL_UNLIT_TEXTURE_DECAL:
        case MATERIAL_UNLIT_TEXTURE_DETAIL:
            uniformTexture0 = shader->FindUniformLocationByName("texture0");
            uniformTexture1 = shader->FindUniformLocationByName("texture1");
            uniformUvOffset = shader->FindUniformLocationByName("uvOffset");
            uniformUvScale = shader->FindUniformLocationByName("uvScale");
            
            break;
        case MATERIAL_VERTEX_LIT_TEXTURE:
            //
            uniformLightPosition0 = shader->FindUniformLocationByName("lightPosition0");
            uniformMaterialLightAmbientColor = shader->FindUniformLocationByName("materialLightAmbientColor");
            uniformMaterialLightDiffuseColor = shader->FindUniformLocationByName("materialLightDiffuseColor");
            uniformMaterialLightSpecularColor = shader->FindUniformLocationByName("materialLightSpecularColor");
            uniformMaterialSpecularShininess = shader->FindUniformLocationByName("materialSpecularShininess");
            uniformLightIntensity0 = shader->FindUniformLocationByName("lightIntensity0");
            uniformLightAttenuationQ = shader->FindUniformLocationByName("uniformLightAttenuationQ");
            
            break;
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
            uniformTexture0 = shader->FindUniformLocationByName("texture0");
            uniformTexture1 = shader->FindUniformLocationByName("normalMapTexture");

            uniformLightPosition0 = shader->FindUniformLocationByName("lightPosition0");
            uniformMaterialLightAmbientColor = shader->FindUniformLocationByName("materialLightAmbientColor");
            uniformMaterialLightDiffuseColor = shader->FindUniformLocationByName("materialLightDiffuseColor");
            uniformMaterialLightSpecularColor = shader->FindUniformLocationByName("materialLightSpecularColor");
            uniformMaterialSpecularShininess = shader->FindUniformLocationByName("materialSpecularShininess");
            uniformLightIntensity0 = shader->FindUniformLocationByName("lightIntensity0");
            uniformLightAttenuationQ = shader->FindUniformLocationByName("uniformLightAttenuationQ");
            break;

        default:
            break;
    };
    
    if (isFogEnabled)
    {
        uniformFogDensity = shader->FindUniformLocationByName("fogDensity");
        uniformFogColor = shader->FindUniformLocationByName("fogColor");
        DVASSERT(uniformFogDensity != -1);
        DVASSERT(uniformFogColor != -1);
    }
    
    if (isFlatColorEnabled)
    {
        uniformFlatColor = shader->FindUniformLocationByName("flatColor");
        DVASSERT(uniformFlatColor != -1);
    }
    
    if (isTexture0ShiftEnabled)
    {
        uniformTexture0Shift = shader->FindUniformLocationByName("texture0Shift");
        DVASSERT(uniformTexture0Shift != -1);
    }
    
    


    //RetrieveTextureSlotNames();
}
    
void Material::RetrieveTextureSlotNames()
{
    // 
    //shader->F
    textureSlotCount = 0;
    for (int32  k = 0; k < shader->GetUniformCount(); ++k)
    {
        if (shader->GetUniformType(k) == Shader::UT_SAMPLER_2D)
        {
            textureSlotNames[textureSlotCount] = shader->GetUniformName(k);
            textureSlotCount++;
        }
    }
}
    
uint32 Material::GetTextureSlotCount() const
{
    return textureSlotCount;
}
    
const String & Material::GetTextureSlotName(uint32 index) const
{
    return textureSlotNames[index];
}
    
uint32 Material::GetTextureSlotIndexByName(const String & string) const
{
    return -1;
}

void Material::SetType(eType _type)
{
    type = _type;
    RebuildShader();
}
    
void Material::Save(KeyedArchive * keyedArchive, SceneFileV2 * sceneFile)
{
    DataNode::Save(keyedArchive, sceneFile);
    
    keyedArchive->SetInt32("mat.texCount", TEXTURE_COUNT);
    for (int32 k = 0; k < TEXTURE_COUNT; ++k)
    {
        if (names[k].Initalized())
        {
            String filename = names[k].GetRelativePath(sceneFile->GetScenePath());
            keyedArchive->SetString(Format("mat.tex%d", k), filename);
            
            if(sceneFile->DebugLogEnabled())
                Logger::Debug("--- save material texture: %s", filename.c_str());
        }
    }

    keyedArchive->SetByteArrayAsType("mat.diffuse", diffuseColor);
    keyedArchive->SetByteArrayAsType("mat.ambient", ambientColor);
    keyedArchive->SetByteArrayAsType("mat.specular", specularColor);
    keyedArchive->SetByteArrayAsType("mat.emission", emissiveColor);
    keyedArchive->SetFloat("mat.shininess", shininess);

    keyedArchive->SetBool("mat.isOpaque", isTranslucent);
    keyedArchive->SetBool("mat.isTwoSided", isTwoSided);

	keyedArchive->SetBool("mat.isAlphablend", isAlphablend);
	keyedArchive->SetInt32("mat.blendSrc", blendSrc);
	keyedArchive->SetInt32("mat.blendDst", blendDst);

    keyedArchive->SetInt32("mat.type", type);
    
    keyedArchive->SetByteArrayAsType("mat.fogcolor", fogColor);
    keyedArchive->SetFloat("mat.fogdencity", fogDensity);
    keyedArchive->SetBool("mat.isFogEnabled", isFogEnabled);

	keyedArchive->SetBool("mat.isFlatColorEnabled", isFlatColorEnabled);
	keyedArchive->SetBool("mat.isTexture0ShiftEnabled", isTexture0ShiftEnabled);
}

void Material::Load(KeyedArchive * keyedArchive, SceneFileV2 * sceneFile)
{
    DataNode::Load(keyedArchive, sceneFile);

    int32 texCount = keyedArchive->GetInt32("mat.texCount");
    for (int32 k = 0; k < texCount; ++k)
    {
        String relativePathname = keyedArchive->GetString(Format("mat.tex%d", k));
        if (!relativePathname.empty())
        {
            if(relativePathname[0] == '~') //path like ~res:/Gfx...
            {
                names[k].InitFromAbsolutePath(relativePathname);
            }
            else
            {
                names[k].InitFromRelativePath(relativePathname, sceneFile->GetScenePath());
            }
            
            if(sceneFile->DebugLogEnabled())
                //Logger::Debug("--- load material texture: %s abs:%s", relativePathname.c_str(), names[k].GetAbsolutePath().c_str());
            	Logger::Debug("--- load material texture: %s src:%s", relativePathname.c_str(), names[k].GetSourcePath().c_str());
            
            //textures[k] = Texture::CreateFromFile(names[k].GetAbsolutePath());
            textures[k] = Texture::CreateFromFile(names[k].GetSourcePath());
        }
    }
    
    
    diffuseColor = keyedArchive->GetByteArrayAsType("mat.diffuse", diffuseColor);
    ambientColor = keyedArchive->GetByteArrayAsType("mat.ambient", ambientColor);
    specularColor = keyedArchive->GetByteArrayAsType("mat.specular", specularColor);
    emissiveColor = keyedArchive->GetByteArrayAsType("mat.emission", emissiveColor);
    shininess = keyedArchive->GetFloat("mat.shininess", shininess);
    
    isTranslucent = keyedArchive->GetBool("mat.isOpaque", isTranslucent);
    isTwoSided = keyedArchive->GetBool("mat.isTwoSided", isTwoSided);

	isAlphablend = keyedArchive->GetBool("mat.isAlphablend", isAlphablend);
	blendSrc = (eBlendMode)keyedArchive->GetInt32("mat.blendSrc", blendSrc);
	blendDst = (eBlendMode)keyedArchive->GetInt32("mat.blendDst", blendDst);

	fogColor = keyedArchive->GetByteArrayAsType("mat.fogcolor", fogColor);
	isFogEnabled = keyedArchive->GetBool("mat.isFogEnabled", isFogEnabled);
	fogDensity = keyedArchive->GetFloat("mat.fogdencity", fogDensity);

	isFlatColorEnabled = keyedArchive->GetBool("mat.isFlatColorEnabled", isFlatColorEnabled);
	isTexture0ShiftEnabled = keyedArchive->GetBool("mat.isTexture0ShiftEnabled", isTexture0ShiftEnabled);

    eType mtype = (eType)keyedArchive->GetInt32("mat.type", type);
    SetType(mtype);
}

void Material::SetOpaque(bool _isOpaque)
{
    isTranslucent = _isOpaque;
    RebuildShader();
}

bool Material::GetOpaque()
{
    return isTranslucent;
}
void Material::SetTwoSided(bool _isTwoSided)
{
    isTwoSided = _isTwoSided;
}
    
bool Material::GetTwoSided()
{
    return isTwoSided;
}
    
void Material::SetAmbientColor(const Color & color)
{
    ambientColor = color;
}
void Material::SetDiffuseColor(const Color & color)
{
    diffuseColor = color;
}
void Material::SetSpecularColor(const Color & color)
{
    specularColor = color;
}
void Material::SetEmissiveColor(const Color & color)
{
    emissiveColor = color;
}

const Color & Material::GetAmbientColor() const
{
    return ambientColor;
}
const Color & Material::GetDiffuseColor() const
{
    return diffuseColor;
}
const Color & Material::GetSpecularColor() const
{
    return specularColor;
}
const Color & Material::GetEmissiveColor() const
{
    return emissiveColor;
}
    
void Material::SetShininess(float32 _shininess)
{
    shininess = _shininess;
}

float32 Material::GetShininess() const
{
    return shininess;
}
    
void Material::SetFog(bool _isFogEnabled)
{
    isFogEnabled = _isFogEnabled;
    RebuildShader();
}

void Material::SetViewOption(eViewOptions option)
{
	if(viewOptions != option)
	{
		viewOptions = option;
		RebuildShader();
	}
}

Material::eViewOptions Material::GetViewOption()
{
	return viewOptions;
}
    
bool Material::IsFogEnabled() const
{
    return isFogEnabled;
}
    
void Material::SetFogDensity(float32 _fogDensity)
{
    fogDensity = _fogDensity;
}
    
float32 Material::GetFogDensity() const
{
    return fogDensity;
}

void Material::SetFogColor(const Color & _fogColor)
{
    fogColor = _fogColor;
}

const Color & Material::GetFogColor() const
{
    return fogColor;
}

void Material::PrepareRenderState(InstanceMaterialState * instanceMaterialState)
{
    ///float32 timeElapsed = SystemTimer::Instance()->FrameDelta();

    if(MATERIAL_UNLIT_TEXTURE_LIGHTMAP == type)
	{
        if (!instanceMaterialState->lightmapTexture)
        {
            SetSetupLightmap(true);
        }
		else
        {
            SetSetupLightmap(false);
            renderStateBlock.SetTexture(instanceMaterialState->lightmapTexture, 1);
        }
    }
	else if (MATERIAL_UNLIT_TEXTURE_DECAL == type || MATERIAL_UNLIT_TEXTURE_DETAIL == type)
    {
        renderStateBlock.SetTexture(textures[Material::TEXTURE_DECAL], 1);
    }


    
	renderStateBlock.shader = shader;

	if (textures[Material::TEXTURE_DIFFUSE])
	{
		renderStateBlock.SetTexture(textures[Material::TEXTURE_DIFFUSE], 0);
	}


	if(MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE == type || MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR == type || MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP == type)
	{
		if (textures[Material::TEXTURE_NORMALMAP])
		{
			renderStateBlock.SetTexture(textures[Material::TEXTURE_NORMALMAP], 1);
		}
	}
	else
	{
//		if (textures[Material::TEXTURE_DECAL])
//		{
//			renderStateBlock.SetTexture(textures[Material::TEXTURE_DECAL], 1);
//		}
	}

	if (isTranslucent || isTwoSided)
	{
		renderStateBlock.state &= ~RenderState::STATE_CULL;
	}
	else
	{
		renderStateBlock.state |= RenderState::STATE_CULL;
	}


	if(isAlphablend)
	{
		renderStateBlock.state |= RenderState::STATE_BLEND;
		//Dizz: temporary solution
		renderStateBlock.state &= ~RenderState::STATE_DEPTH_WRITE;

		renderStateBlock.SetBlendMode((eBlendMode)blendSrc, (eBlendMode)blendDst);
	}
	else
	{
		//Dizz: temporary solution
		renderStateBlock.state |= RenderState::STATE_DEPTH_WRITE;
		renderStateBlock.state &= ~RenderState::STATE_BLEND;
	}

	if(isWireframe)
	{
		renderStateBlock.SetFillMode(FILLMODE_WIREFRAME);
	}
	else
	{
		renderStateBlock.SetFillMode(FILLMODE_SOLID);
	}

	// render
	RenderManager::Instance()->FlushState(&renderStateBlock);
	RenderManager::Instance()->AttachRenderData();


    if(uniformTexture0 != -1)
    {
        shader->SetUniformValue(uniformTexture0, 0);
    }
    if(uniformTexture1 != -1)
    {
        shader->SetUniformValue(uniformTexture1, 1);
    }

	if(isSetupLightmap)
	{
		int32 lightmapSizePosition = shader->FindUniformLocationByName("lightmapSize");
		if (lightmapSizePosition != -1)
		{
			shader->SetUniformValue(lightmapSizePosition, (float32)setupLightmapSize); 
		}
	}
    

	if(MATERIAL_UNLIT_TEXTURE_LIGHTMAP == type)
	{
		if (uniformUvOffset != -1)
			shader->SetUniformValue(uniformUvOffset, instanceMaterialState->uvOffset);
		if (uniformUvScale != -1)
			shader->SetUniformValue(uniformUvScale, instanceMaterialState->uvScale);
	}

	if (isFogEnabled)
	{
		DVASSERT(uniformFogDensity != -1);
        shader->SetUniformValue(uniformFogDensity, fogDensity);
		
        DVASSERT(uniformFogColor != -1)
        shader->SetUniformColor3(uniformFogColor, fogColor);
	}
    
    if (instanceMaterialState)
    {
        if (isFlatColorEnabled)
        {
            DVASSERT(uniformFlatColor != -1);
            shader->SetUniformColor4(uniformFlatColor, instanceMaterialState->flatColor);
        }
        if (isTexture0ShiftEnabled)
        {
            DVASSERT(uniformTexture0Shift != -1);
            shader->SetUniformValue(uniformTexture0Shift, instanceMaterialState->texture0Shift);
        }

		if(scene)
		{
			Camera * camera = scene->GetCurrentCamera();
			Light * lightNode0 = instanceMaterialState->GetLight(0);
			if (lightNode0 && camera)
			{
				if (uniformLightPosition0 != -1)
				{
					const Matrix4 & matrix = camera->GetMatrix();
					Vector3 lightPosition0InCameraSpace = lightNode0->GetPosition() * matrix;
                
					shader->SetUniformValue(uniformLightPosition0, lightPosition0InCameraSpace);
				}
				if (uniformMaterialLightAmbientColor != -1)
				{
					shader->SetUniformColor3(uniformMaterialLightAmbientColor, lightNode0->GetAmbientColor() * GetAmbientColor());
				}
				if (uniformMaterialLightDiffuseColor != -1)
				{
					shader->SetUniformColor3(uniformMaterialLightDiffuseColor, lightNode0->GetDiffuseColor() * GetDiffuseColor());
				}
				if (uniformMaterialLightSpecularColor != -1)
				{
					shader->SetUniformColor3(uniformMaterialLightSpecularColor, lightNode0->GetSpecularColor() * GetSpecularColor());
				}
				if (uniformMaterialSpecularShininess != -1)
				{
					shader->SetUniformValue(uniformMaterialSpecularShininess, shininess);
				}
            
				if (uniformLightIntensity0 != -1)
				{
					shader->SetUniformValue(uniformLightIntensity0, lightNode0->GetIntensity());
				}
				if (uniformLightAttenuationQ != -1)
				{
					//shader->SetUniformValue(uniformLightAttenuationQ, lightNode0->GetAttenuation());
				}
			}
		}
    }

}

void Material::Draw(PolygonGroup * group, InstanceMaterialState * instanceMaterialState)
{
	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::MATERIAL_DRAW))
	{
		return;
	}

	if(isTranslucent && !RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::TRANSPARENT_DRAW))
	{
		return;
	}

	if(!isTranslucent && !RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::OPAQUE_DRAW))
	{
		return;
	}

	//Dizz: uniformFogDensity != -1 is a check if fog is inabled in shader
	if(isFogEnabled && (uniformFogDensity != -1) && !RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
	{
		RebuildShader();
	}

	if(isFogEnabled && (uniformFogDensity == -1) && RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
	{
		RebuildShader();
	}

	RenderManager::Instance()->SetRenderData(group->renderDataObject);

	eBlendMode oldSrc;
	eBlendMode oldDst;
	if(isAlphablend)
	{
		oldSrc = RenderManager::Instance()->GetSrcBlend();
		oldDst = RenderManager::Instance()->GetDestBlend();
	}

	PrepareRenderState(instanceMaterialState);

    // TODO: rethink this code
    if (group->renderDataObject->GetIndexBufferID() != 0)
    {
        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, group->indexCount, EIF_16, 0);
    }
    else
    {
        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, group->indexCount, EIF_16, group->indexArray);
    }

    
	//RenderManager::Instance()->SetTexture(0, 1); 
	//RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
	//if(isAlphablend)
	//{
	//	RenderManager::Instance()->SetBlendMode(oldSrc, oldDst);
	//}
}



void Material::SetSetupLightmap(bool _isSetupLightmap)
{
	if(isSetupLightmap != _isSetupLightmap)
	{
		isSetupLightmap = _isSetupLightmap;
		RebuildShader();
	}
}

bool Material::GetSetupLightmap()
{
	return isSetupLightmap;
}

void Material::SetSetupLightmapSize(int32 _setupLightmapSize)
{
	setupLightmapSize = _setupLightmapSize;
}
    
void Material::SetTexture(eTextureLevel level, Texture * texture)
{
    if (texture == textures[level])return;
    
    SafeRelease(textures[level]);
//	names[level] = String("");
	names[level].InitFromAbsolutePath(String(""));

    textures[level] = SafeRetain(texture);
	if(textures[level])
	{
		if(!textures[level]->isRenderTarget)
		{
//			names[level] = textures[level]->GetPathname();
			names[level].InitFromAbsolutePath(textures[level]->GetPathname());
		}
	}
}

void Material::SetTexture(eTextureLevel level, const String & textureName)
{
    SafeRelease(textures[level]);
//    names[level] = "";
	names[level].InitFromAbsolutePath(String(""));
 
    Texture *t = Texture::CreateFromFile(textureName);
    if(t)
    {
        textures[level] = t;
//        names[level] = textureName;
        names[level].InitFromAbsolutePath(textureName);
    }
}

void Material::SetAlphablend(bool _isAlphablend)
{
	isAlphablend = _isAlphablend;
	RebuildShader();
}

bool Material::GetAlphablend()
{
	return isAlphablend;
}

RenderState * Material::GetRenderState()
{
	return &renderStateBlock;
}

void Material::SetWireframe(bool _isWireframe)
{
    isWireframe = _isWireframe;
}
    
bool Material::GetWireframe()
{
    return isWireframe;
}

void Material::EnableFlatColor(const bool & isEnabled)
{
    isFlatColorEnabled = isEnabled;
    RebuildShader();
    
}

const bool & Material::IsFlatColorEnabled()
{
    return isFlatColorEnabled;
}

void Material::EnableTextureShift(const bool & isEnabled)
{
    isTexture0ShiftEnabled = isEnabled;
    RebuildShader();
    
}

const bool & Material::IsTextureShiftEnabled()
{
    return isTexture0ShiftEnabled;
}

    
const FastName & Material::GetOwnerLayerName()
{
    if(GetAlphablend())
    {
        SetOwnerLayerName(LAYER_TRANSLUCENT);
    }
    else SetOwnerLayerName(LAYER_OPAQUE);

    return ownerLayerName;
}

void Material::SetOwnerLayerName(const FastName & fastname)
{
    ownerLayerName = fastname;
    
}
const bool & Material::IsExportOwnerLayerEnabled()
{
    return isExportOwnerLayerEnabled;
}
    
void Material::SetExportOwnerLayer(const bool & isEnabled)
{
    isExportOwnerLayerEnabled = isEnabled;
}


};
