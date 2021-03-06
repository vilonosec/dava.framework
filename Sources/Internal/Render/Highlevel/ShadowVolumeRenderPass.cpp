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
#include "Render/Highlevel/ShadowVolumeRenderPass.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/ShadowRect.h"

namespace DAVA
{
    
ShadowVolumeRenderPass::ShadowVolumeRenderPass(const FastName & _name)
    :   RenderPass(_name)
    ,   shadowRect(0)
{
    
}

ShadowVolumeRenderPass::~ShadowVolumeRenderPass()
{
    SafeRelease(shadowRect);
}

void ShadowVolumeRenderPass::Draw(Camera * camera)
{
    // Draw all layers with their materials
    uint32 size = (uint32)renderLayers.size();
    DVASSERT(size == 1);
    
    RenderLayer * shadowVolumesLayer = renderLayers[0];
    //Vector<LightNode*> & lights = renderSystem->GetLights();
    uint32 renderBatchCount = shadowVolumesLayer->GetRenderBatchCount();
    
    if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SHADOWVOLUME_DRAW)
       && renderBatchCount > 0)
	{
		if(!shadowRect)
		{
			shadowRect = ShadowRect::Create();
		}
        
		//2nd pass
		RenderManager::Instance()->RemoveState(RenderState::STATE_CULL);
		RenderManager::Instance()->RemoveState(RenderState::STATE_DEPTH_WRITE);
		RenderManager::Instance()->AppendState(RenderState::STATE_BLEND);
		RenderManager::Instance()->SetBlendMode(BLEND_ZERO, BLEND_ONE);
        
		RenderManager::Instance()->ClearStencilBuffer(0);
		RenderManager::Instance()->AppendState(RenderState::STATE_STENCIL_TEST);
		
		RenderManager::State()->SetStencilFunc(FACE_FRONT_AND_BACK, CMP_ALWAYS);
		RenderManager::State()->SetStencilRef(1);
		RenderManager::State()->SetStencilMask(0xFFFFFFFF);
        
		RenderManager::State()->SetStencilPass(FACE_FRONT, STENCILOP_KEEP);
		RenderManager::State()->SetStencilFail(FACE_FRONT, STENCILOP_KEEP);
		RenderManager::State()->SetStencilZFail(FACE_FRONT, STENCILOP_DECR_WRAP);
        
		RenderManager::State()->SetStencilPass(FACE_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilFail(FACE_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilZFail(FACE_BACK, STENCILOP_INCR_WRAP);
		
		RenderManager::Instance()->FlushState();
    
        shadowVolumesLayer->Draw(camera);
        
		//3rd pass
		RenderManager::Instance()->RemoveState(RenderState::STATE_CULL);
		RenderManager::Instance()->RemoveState(RenderState::STATE_DEPTH_TEST);
		
		RenderManager::State()->SetStencilRef(0);
		RenderManager::State()->SetStencilFunc(FACE_FRONT_AND_BACK, CMP_NOTEQUAL);
		RenderManager::State()->SetStencilPass(FACE_FRONT_AND_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilFail(FACE_FRONT_AND_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilZFail(FACE_FRONT_AND_BACK, STENCILOP_KEEP);
        
		RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		shadowRect->Draw();
	}
}
    
ShadowRect * ShadowVolumeRenderPass::GetShadowRect() const
{
    return shadowRect;
}

    
};
