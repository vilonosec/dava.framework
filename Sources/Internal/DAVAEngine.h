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
#ifndef __DAVAENGINE_H__
#define __DAVAENGINE_H__

#include "DAVAConfig.h"
#include "Debug/MemoryManager.h"
#include "Debug/Stats.h"
#include "Debug/Backtrace.h"

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Utils/StringFormat.h"

#include "DLC/DLCSystem.h"
#include "DLC/DLCUnpacker.h"
#include "DLC/FileDownloader.h"

#include "FileSystem/Logger.h"
#include "Platform/SystemTimer.h"

// system stuff
#include "Utils/Utils.h"
#include "Utils/UTF8Utils.h"
#include "Utils/MD5.h"
#include "Base/Message.h"
#include "Base/BaseObject.h"
#include "Debug/DVAssert.h"
#include "Base/Singleton.h"
#include "Utils/StringFormat.h"
#include "UI/ScrollHelper.h"
#include "Debug/Replay.h"
#include "Utils/Random.h"

#include "Base/ObjectFactory.h"
#include "Base/FixedSizePoolAllocator.h"
#include "Base/HashMap.h"

// ptrs
#include "Base/RefPtr.h"
#include "Base/ScopedPtr.h"

// threads
#include "Platform/Thread.h"
#include "Platform/Mutex.h"

// Accelerometer
#include "Input/Accelerometer.h"

#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

// Localization
#include "FileSystem/LocalizationSystem.h"

// Image formats stuff (PNG & JPG & other formats)
#include "Render/LibPngHelpers.h"
#include "Render/Image.h"

// Files & Serialization
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/FileList.h"
#include "FileSystem/VariantType.h"
#include "FileSystem/KeyedArchiver.h"
#include "FileSystem/KeyedUnarchiver.h"
#include "FileSystem/KeyedArchive.h"

#include "FileSystem/XMLParser.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/Parser.h"


// Collisions
#include "Collision/Collisions.h"

// Animation manager
#include "Animation/Interpolation.h"
#include "Animation/AnimatedObject.h"
#include "Animation/Animation.h"
#include "Animation/AnimationManager.h"
#include "Animation/LinearAnimation.h"
#include "Animation/BezierSplineAnimation.h"


// 2D Graphics
#include "Render/2D/Sprite.h"
#include "Render/TextureDescriptor.h"
#include "Render/Texture.h"
#include "Render/Image.h"
#include "Render/ImageLoader.h"
#include "Render/Shader.h"

#include "Core/DisplayMode.h"
#include "Render/RenderManager.h"

#include "Render/RenderEffect.h"
#include "Render/RenderGrayscaleEffect.h"

#include "Render/RenderHelper.h"

#include "Render/Cursor.h"

#include "Render/LibDxtHelper.h"


// Fonts
#include "Render/2D/Font.h"
#include "Render/2D/GraphicsFont.h"
#include "Render/2D/FTFont.h"
#include "Render/2D/FontManager.h"
#include "Render/2D/TextBlock.h"

// UI
#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/UIEvent.h"
#include "UI/UIButton.h"
#include "UI/UIStaticText.h"
#include "UI/UIControlBackground.h"
#include "UI/UIScreen.h"
#include "UI/UIList.h"
#include "UI/UIListCell.h"
#include "UI/UIJoypad.h"
#include "UI/UITextField.h"
#include "UI/UISlider.h"
#include "UI/UIScrollBar.h"
#include "UI/UIJoypad.h"
#include "UI/UI3DView.h"
#include "UI/UIHierarchy.h"
#include "UI/UIHierarchyCell.h"
#include "UI/UIFileSystemDialog.h"
#include "UI/UIWebView.h"
#include "UI/UISpinner.h"
#include "UI/VectorSpinnerAdapter.h"
#include "UI/UISwitch.h"

#include "UI/UIYamlLoader.h"

#include "UI/UIScreenTransition.h"
#include "UI/UIMoveInTransition.h"
#include "UI/UIFadeTransition.h"
#include "UI/UIHoleTransition.h"

#include "UI/UIScreenManager.h"

#include "UI/TheoraPlayer.h"
#include "UI/UIAggregatorControl.h"


// Game object manager / 2D Scene
#include "Scene2D/GameObject.h"
#include "Scene2D/GameObjectManager.h"
#include "Collision/CollisionObject2.h"
#include "Scene2D/Box2DGameObjectManager.h"
#include "Scene2D/Box2DGameObject.h"
#include "Scene2D/Box2DTileCollider.h"
#include "Scene2D/Box2DHelper.h"

// Sound & Music
#include "Sound/Sound.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundInstance.h"
#include "Sound/SoundGroup.h"

// Particle System
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLayer.h"
#include "Particles/Particle.h"
#include "Particles/ParticleEmitterObject.h"
#include "Particles/ParticleEmitter3D.h"
#include "Particles/ParticleLayer3D.h"
#include "Particles/ParticleLayerLong.h"


// 3D core classes
#include "Scene3D/SceneFile.h"
#include "Scene3D/SceneFileV2.h"

#include "Render/3D/StaticMesh.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/3D/EdgeAdjacency.h"

// Material compiler
#include "Render/Material/MaterialCompiler.h"
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialGraphNode.h"

// 3D scene management
#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SpriteNode.h"
#include "Scene3D/MeshInstanceNode.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/Heightmap.h"
#include "Render/Highlevel/Light.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/ShadowVolume.h"
#include "Render/Highlevel/SpriteObject.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/RenderFastNames.h"

#include "Scene3D/ShadowVolumeNode.h"
#include "Scene3D/LodNode.h"
#include "Scene3D/ImposterNode.h"
#include "Scene3D/ParticleEmitterNode.h"
#include "Scene3D/ParticleEffectNode.h"
#include "Scene3D/SwitchNode.h"
#include "Scene3D/UserNode.h"
#include "Scene3D/Systems/LodSystem.h"
#include "Scene3D/Systems/EventSystem.h"

//Components
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/BulletComponent.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/UpdatableComponent.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/UserComponent.h"

// Application core 
#include "Core/Core.h"
#include "Core/ApplicationCore.h"

// Networking
#include "Network/NetworkConnection.h"
#include "Network/NetworkDelegate.h"
#include "Network/NetworkPacket.h"

#endif // __DAVAENGINE_H__

