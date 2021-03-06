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
#include "Render/Highlevel/SpriteObject.h"
#include "Render/Highlevel/SpriteRenderBatch.h"


namespace DAVA 
{

SpriteObject::SpriteObject(const String &pathToSprite, int32 _frame
							, const Vector2 &reqScale, const Vector2 &pivotPoint)
	:   RenderObject()
{
	Sprite *spr = Sprite::Create(pathToSprite);
	Init(spr, _frame, reqScale, pivotPoint);
	SafeRelease(spr);
}

SpriteObject::SpriteObject(Sprite *spr, int32 _frame
							, const Vector2 &reqScale, const Vector2 &pivotPoint)
	:   RenderObject()
{
	Init(spr, _frame, reqScale, pivotPoint);
}


SpriteObject::~SpriteObject()
{
	SafeRelease(sprite);
}

void SpriteObject::Init( Sprite *spr, int32 _frame, const Vector2 &reqScale, const Vector2 &pivotPoint )
{
	type = TYPE_SPRITE;

	spriteType = SPRITE_OBJECT;

	sprScale = reqScale;
	sprPivot = pivotPoint;
	sprite = SafeRetain(spr);
	frame = _frame;

	SetupRenderBatch();
}

void SpriteObject::SetupRenderBatch()
{
	if(sprite)
	{
		for (int32 i = 0; i < sprite->GetFrameCount(); ++i) 
		{
			CreateMeshFromSprite(i);
		}
	}

	RenderDataObject *renderDataObject = new RenderDataObject();
	renderDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &verts.front());
	renderDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &textures.front());

	Material * material = new Material();
	material->SetType(Material::MATERIAL_UNLIT_TEXTURE);
	material->SetAlphablend(true);
	material->SetBlendSrc(BLEND_SRC_ALPHA);
	material->SetBlendDest(BLEND_ONE_MINUS_SRC_ALPHA);
	material->SetName("SpriteObject_material");

	material->GetRenderState()->SetTexture(sprite->GetTexture(frame));

	SpriteRenderBatch *batch = new SpriteRenderBatch();
	batch->SetMaterial(material);
	batch->SetRenderDataObject(renderDataObject);
	AddRenderBatch(batch);

	SafeRelease(material);
	SafeRelease(renderDataObject);
	SafeRelease(batch);
}


RenderObject * SpriteObject::Clone(RenderObject *newObject)
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<SpriteObject>(this), "Can clone only SpriteObject");

		SpriteObject *newObject = new SpriteObject(sprite, frame, sprScale, sprPivot);
		newObject->spriteType = spriteType;
	}

	return RenderObject::Clone(newObject);
}


void SpriteObject::SetFrame(int32 newFrame)
{
	frame = Clamp(newFrame, 0, sprite->GetFrameCount() - 1);

	int32 count = GetRenderBatchCount();
	if(count)
	{
		GetRenderBatch(0)->GetMaterial()->GetRenderState()->SetTexture(sprite->GetTexture(frame));
	}
}

int32 SpriteObject::GetFrame() const
{
	return frame;
}


Sprite * SpriteObject::GetSprite() const
{
	return sprite;
}

void SpriteObject::SetSpriteType(eSpriteType _type)
{
	spriteType = _type;
}

SpriteObject::eSpriteType SpriteObject::GetSpriteType() const
{
	return spriteType;
}

const Vector2 & SpriteObject::GetScale() const
{
	return sprScale;
}

const Vector2 & SpriteObject::GetPivot() const
{
	return sprPivot;
}


void SpriteObject::CreateMeshFromSprite(int32 frameToGen)
{
	float32 x0 = sprite->GetRectOffsetValueForFrame(frameToGen, Sprite::X_OFFSET_TO_ACTIVE) - sprPivot.x;
	float32 y0 = sprite->GetRectOffsetValueForFrame(frameToGen, Sprite::Y_OFFSET_TO_ACTIVE) - sprPivot.y;
	float32 x1 = x0 + sprite->GetRectOffsetValueForFrame(frameToGen, Sprite::ACTIVE_WIDTH);
	float32 y1 = y0 + sprite->GetRectOffsetValueForFrame(frameToGen, Sprite::ACTIVE_HEIGHT);
	x0 *= sprScale.x;
	x1 *= sprScale.y;
	y0 *= sprScale.x;
	y1 *= sprScale.y;

	//triangle 1
	//0, 0
	float32 *pT = sprite->GetTextureVerts(frameToGen);

	verts.push_back(x0);
	verts.push_back(y0);
	verts.push_back(0);
	//textures.push_back(pT[2 * 2 + 0]);
	//textures.push_back(pT[2 * 2 + 1]);


	//1, 0
	verts.push_back(x1);
	verts.push_back(y0);
	verts.push_back(0);
	//textures.push_back(pT[3 * 2 + 0]);
	//textures.push_back(pT[3 * 2 + 1]);


	//0, 1
	verts.push_back(x0);
	verts.push_back(y1);
	verts.push_back(0);
	//textures.push_back(pT[0 * 2 + 0]);
	//textures.push_back(pT[0 * 2 + 1]);

	//1, 1
	verts.push_back(x1);
	verts.push_back(y1);
	verts.push_back(0);
	//textures.push_back(pT[1 * 2 + 0]);
	//textures.push_back(pT[1 * 2 + 1]);


	for (int32 i = 0; i < 2*4; i++) 
	{
		textures.push_back(*pT);
		pT++;
	}
}


};
