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
#ifndef __DAVAENGINE_LANDSCAPE_DEBUG_NODE_H__
#define __DAVAENGINE_LANDSCAPE_DEBUG_NODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Scene3D/SceneNode.h"
#include "Scene3D/Frustum.h"
#include "Scene3D/LandscapeNode.h"

namespace DAVA
{

class Scene;
class Image;
class Texture;
class RenderDataObject;
class Shader;
class SceneFileV2;
class Heightmap;


/**    
    \brief Implementation of cdlod algorithm to render landscapes
    This class is base of the landscape code on all platforms
    Landscape node is always axial aligned for simplicity of frustum culling calculations
    Keep in mind that landscape orientation cannot be changed using localTransform and worldTransform matrices. 
 */ 

class LandscapeDebugNode : public LandscapeNode
{
public:	
	LandscapeDebugNode();
	virtual ~LandscapeDebugNode();
    
    
    virtual void SetDebugHeightmapImage(Heightmap * _debugHeightmapImage, const AABBox3 & _box);
  
    /**
        \brief Overloaded draw function to draw landscape.
     */
	virtual void Draw();

    void SetHeightmapPath(const String &path);
    
    void RebuildVertexes(const Rect &rebuildAtRect);
    
protected:	
    void RebuildIndexes();
    
    void DrawLandscape();
    
    Vector<LandscapeVertex> debugVertices;
    Vector<uint32> debugIndices;
    RenderDataObject * debugRenderDataObject;
};

    
};

#endif // __DAVAENGINE_LANDSCAPE_NODE_H__




