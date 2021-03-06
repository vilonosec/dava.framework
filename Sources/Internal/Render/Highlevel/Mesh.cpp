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
#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/Highlevel/ShadowVolume.h"
namespace DAVA
{

REGISTER_CLASS(Mesh);

Mesh::Mesh()
{
    type = TYPE_MESH;
}
    
Mesh::~Mesh()
{
    
}

void Mesh::AddPolygonGroup(PolygonGroup * polygonGroup, Material * material)
{
    RenderBatch * batch = new RenderBatch();
    batch->SetPolygonGroup(polygonGroup);
    batch->SetMaterial(material);
    batch->SetRenderDataObject(polygonGroup->renderDataObject);
    batch->SetStartIndex(0);
    batch->SetIndexCount(polygonGroup->GetIndexCount());
    AddRenderBatch(batch);
    
	batch->Release();
    //polygonGroups.push_back(polygonGroup);
}
    
uint32 Mesh::GetPolygonGroupCount()
{
    return (uint32)renderBatchArray.size();
}

PolygonGroup * Mesh::GetPolygonGroup(uint32 index)
{
    return renderBatchArray[index]->GetPolygonGroup();
}

RenderObject * Mesh::Clone( RenderObject *newObject )
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<Mesh>(this), "Can clone only Mesh");
		newObject = new Mesh();
	}

	return RenderObject::Clone(newObject);
}

void Mesh::Save(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	RenderObject::Save(archive, sceneFile);
}

void Mesh::Load(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	RenderObject::Load(archive, sceneFile);
}

void Mesh::BakeTransform(const Matrix4 & transform)
{
	uint32 size = renderBatchArray.size();
	for(uint32 i = 0; i < size; ++i)
	{
		PolygonGroup * pg = renderBatchArray[i]->GetPolygonGroup();
		DVASSERT(pg);
		pg->ApplyMatrix(transform);
		pg->BuildBuffers();

		renderBatchArray[i]->UpdateAABBoxFromSource();
	}

	RecalcBoundingBox();
}

ShadowVolume * Mesh::CreateShadow()
{
	DVASSERT(renderBatchArray.size() == 1);

	ShadowVolume * newShadowVolume = new ShadowVolume();
	newShadowVolume->MakeShadowVolumeFromPolygonGroup(GetRenderBatch(0)->GetPolygonGroup());

	return newShadowVolume;
}

};
