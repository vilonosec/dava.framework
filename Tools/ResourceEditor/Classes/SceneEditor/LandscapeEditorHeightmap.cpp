#include "LandscapeEditorHeightmap.h"

#include "LandscapeTool.h"
#include "LandscapeToolsPanelHeightmap.h"
#include "PropertyControlCreator.h"

#include "../EditorScene.h"

#include "HeightmapNode.h"

#include "../LandscapeEditor/EditorHeightmap.h"
#include "../LandscapeEditor/EditorLandscape.h"
#include "../LandscapeEditor/LandscapeRenderer.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../LandscapeEditor/LandscapesController.h"
#include "../Qt/Main/QtUtils.h"

#include "EditorSettings.h"
#include "Scene3D/Components/DebugRenderComponent.h"

#include "../Commands/CommandsManager.h"
#include "../Commands/HeightmapEditorCommands.h"


LandscapeEditorHeightmap::LandscapeEditorHeightmap(LandscapeEditorDelegate *newDelegate, 
                                           EditorBodyControl *parentControl, const Rect &toolsRect)
    :   LandscapeEditorBase(newDelegate, parentControl)
{
    landscapesController = NULL;
    
    toolImage = NULL;
    
    toolsPanel = new LandscapeToolsPanelHeightmap(this, toolsRect);
    
    prevToolSize = 0.f;
    
    editingIsEnabled = false;
    
    copyFromCenter = Vector2(-1.0f, -1.0f);
    copyToCenter = Vector2(-1.0f, -1.0f);
    
    tilemaskWasChanged = false;
    tilemaskImage = NULL;
    tilemaskPathname = "";
    tilemaskTexture = NULL;
    toolImageTile = NULL;

	oldHeightmap = NULL;
	oldTilemap = NULL;
}


LandscapeEditorHeightmap::~LandscapeEditorHeightmap()
{
    SafeRelease(tilemaskImage);
    SafeRelease(toolImageTile);
    SafeRelease(tilemaskTexture);

    SafeRelease(toolImage);
    
    SafeRelease(landscapesController);
	SafeRelease(oldHeightmap);
	SafeRelease(oldTilemap);
}

void LandscapeEditorHeightmap::Update(float32 timeElapsed)
{
    if(editingIsEnabled)
    {
        if(currentTool)
        {
            if((LandscapeTool::TOOL_BRUSH == currentTool->type) || (LandscapeTool::TOOL_COPYPASTE == currentTool->type))
            {
                UpdateTileMaskTool(timeElapsed);
            }
            else if(LandscapeTool::TOOL_DROPPER == currentTool->type)
            {
                currentTool->height = GetDropperHeight();
            }
        }
    }
    
    LandscapeEditorBase::Update(timeElapsed);
}

void LandscapeEditorHeightmap::UpdateToolImage()
{
    if(toolImage && currentTool)
    {
        if(prevToolSize != currentTool->size)
        {
            SafeRelease(toolImage);
            if(LandscapeTool::TOOL_COPYPASTE == currentTool->type)
            {
                SafeRelease(toolImageTile);
            }
        }
    }

    if(currentTool && !toolImage)
    {
        prevToolSize = currentTool->size;

        int32 sideSize = (int32)currentTool->size;
        toolImage = CreateToolImage(sideSize);
        
        if(LandscapeTool::TOOL_COPYPASTE == currentTool->type && tilemaskImage)
        {
            float32 multiplier = (float32)tilemaskImage->GetWidth() / (float32)(landscapeSize);
            sideSize = (int32)(currentTool->size * multiplier);
            toolImageTile = CreateToolImage(sideSize);
        }
    }
}

Image *LandscapeEditorHeightmap::CreateToolImage(int32 sideSize)
{
    RenderManager::Instance()->LockNonMain();
    
    Image *image = currentTool->image;
    Sprite *dstSprite = Sprite::CreateAsRenderTarget((float32)sideSize, (float32)sideSize, FORMAT_RGBA8888);
    Texture *srcTex = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), 
                                              image->GetWidth(), image->GetHeight(), false);
    Sprite *srcSprite = Sprite::CreateFromTexture(srcTex, 0, 0, (float32)image->GetWidth(), (float32)image->GetHeight());
    
    RenderManager::Instance()->SetRenderTarget(dstSprite);
    
    RenderManager::Instance()->SetColor(Color::Black());
    RenderHelper::Instance()->FillRect(Rect(0, 0, (float32)dstSprite->GetTexture()->GetWidth(), (float32)dstSprite->GetTexture()->GetHeight()));
    
    RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
    RenderManager::Instance()->SetColor(Color::White());
    
    srcSprite->SetScaleSize((float32)sideSize, (float32)sideSize);
    srcSprite->SetPosition(Vector2((dstSprite->GetTexture()->GetWidth() - sideSize)/2.0f, 
                                   (dstSprite->GetTexture()->GetHeight() - sideSize)/2.0f));
    srcSprite->Draw();
    RenderManager::Instance()->RestoreRenderTarget();
    
    Image *retImage = dstSprite->GetTexture()->CreateImageFromMemory();
    
    SafeRelease(srcSprite);
    SafeRelease(srcTex);
    SafeRelease(dstSprite);
    
    RenderManager::Instance()->UnlockNonMain();
    
    return retImage;
}


void LandscapeEditorHeightmap::UpdateTileMaskTool(float32 timeElapsed)
{
    if(toolImage)
    {
        if(LandscapeTool::TOOL_BRUSH == currentTool->type)
        {
            UpdateBrushTool(timeElapsed);
        }
        else if(LandscapeTool::TOOL_COPYPASTE == currentTool->type)
        {
            UpdateCopypasteTool(timeElapsed);
        }
    }
}

void LandscapeEditorHeightmap::UpdateBrushTool(float32 timeElapsed)
{
    int32 scaleSize = toolImage->GetWidth();
    Vector2 pos = landscapePoint - Vector2((float32)scaleSize, (float32)scaleSize)/2.0f;
    {
        EditorHeightmap *heightmap = dynamic_cast<EditorHeightmap *>(landscapesController->GetCurrentHeightmap());
        DVASSERT(heightmap);
        
        if(currentTool->averageDrawing)
        {
            float32 koef = (currentTool->averageStrength * timeElapsed) * 2.0f;
            heightmap->DrawAverageRGBA(toolImage, (int32)pos.x, (int32)pos.y, scaleSize, scaleSize, koef);
        }
        else if(currentTool->relativeDrawing)
        {
            float32 koef = (currentTool->strength * timeElapsed);
            if(inverseDrawingEnabled)
            {
                koef = -koef;
            }
            heightmap->DrawRelativeRGBA(toolImage, (int32)pos.x, (int32)pos.y, scaleSize, scaleSize, koef);
        }
        else
        {
            Vector3 landSize;
            AABBox3 transformedBox;
            workingLandscape->GetBoundingBox().GetTransformedBox(*workingLandscape->GetWorldTransformPtr(), transformedBox);
            landSize = transformedBox.max - transformedBox.min;
            
            float32 maxHeight = landSize.z;
            float32 height = currentTool->height / maxHeight * Heightmap::MAX_VALUE;
            
            float32 koef = (currentTool->averageStrength * timeElapsed) * 2.0f;
            heightmap->DrawAbsoluteRGBA(toolImage, (int32)pos.x, (int32)pos.y, scaleSize, scaleSize, koef, height);
        }
        
        UpdateHeightmap(Rect(pos.x, pos.y, (float32)scaleSize, (float32)scaleSize));
    }
}

void LandscapeEditorHeightmap::UpdateCopypasteTool(float32 timeElapsed)
{
    if(     (Vector2(-1.0f, -1.0f) != copyFromCenter) 
       &&   (Vector2(-1.0f, -1.0f) != copyToCenter))
    {
        if(currentTool->copyTilemask)
        {
            if(tilemaskImage && toolImageTile)
            {
                tilemaskWasChanged = true;
                
                float32 multiplier = (float32)tilemaskImage->GetWidth() / (float32)(landscapeSize);
                
                int32 scaleSize = toolImageTile->GetWidth();
                Vector2 posTo = landscapePoint * multiplier - Vector2((float32)scaleSize, (float32)scaleSize)/2.f;
                
                Vector2 deltaPos = landscapePoint - copyToCenter;
                Vector2 posFrom = (copyFromCenter + deltaPos) * multiplier - Vector2((float32)scaleSize, (float32)scaleSize)/2.f;
                
                EditorHeightmap::DrawCopypasteRGBA(tilemaskImage, tilemaskImage, toolImageTile, posFrom, posTo, scaleSize, scaleSize);
                
                Texture *tex = tilemaskTexture;
                if(tex)
                {
                    tex->TexImage(0, tilemaskImage->GetWidth(), tilemaskImage->GetHeight(), tilemaskImage->GetData(), 0);
                    //TODO: is code useful?
                    tex->GenerateMipmaps();
                    tex->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);
                    //ENDOFTODO
                    
                    workingLandscape->UpdateFullTiledTexture();
                }
            }
        }

        
        if(currentTool->copyHeightmap)
        {
            int32 scaleSize = toolImage->GetWidth();
            Vector2 posTo = landscapePoint - Vector2((float32)scaleSize, (float32)scaleSize)/2.f;
            
            Vector2 deltaPos = landscapePoint - copyToCenter;
            Vector2 posFrom = copyFromCenter + deltaPos - Vector2((float32)scaleSize, (float32)scaleSize)/2.f;
            
            float32 koef = (currentTool->averageStrength * timeElapsed) * 2.0f;

            EditorHeightmap *heightmap = dynamic_cast<EditorHeightmap *>(landscapesController->GetCurrentHeightmap());
            DVASSERT(heightmap);
            heightmap->DrawCopypasteRGBA(toolImage, posFrom, posTo, scaleSize, scaleSize, koef);
            
            UpdateHeightmap(Rect(posTo.x, posTo.y, (float32)scaleSize, (float32)scaleSize));
        }
    }
}


float32 LandscapeEditorHeightmap::GetDropperHeight()
{
    Vector3 landSize;
    AABBox3 transformedBox;
    workingLandscape->GetBoundingBox().GetTransformedBox(*workingLandscape->GetWorldTransformPtr(), transformedBox);
    landSize = transformedBox.max - transformedBox.min;

    Heightmap *heightmap = landscapesController->GetCurrentHeightmap();
    int32 index = (int32)(landscapePoint.x + landscapePoint.y * heightmap->Size());
    float32 height = heightmap->Data()[index];
    float32 maxHeight = landSize.z;
    return (height / Heightmap::MAX_VALUE * maxHeight);
}

void LandscapeEditorHeightmap::UpdateCursor()
{
	if(currentTool)
	{
		float32 scaleSize = (float32)((int32)currentTool->size);
		Vector2 pos = landscapePoint - Vector2(scaleSize, scaleSize)/2.f;

        Landscape *landscape = landscapesController->GetCurrentLandscape();
		landscape->SetCursorTexture(cursorTexture);
		landscape->SetBigTextureSize((float32)landscapeSize);
		landscape->SetCursorPosition(pos);
		landscape->SetCursorScale(scaleSize);
	}
}

void LandscapeEditorHeightmap::InputAction(int32 phase, bool intersects)
{
    bool dropper = IsKeyModificatorPressed(DVKEY_CTRL);
    if(dropper)
    {
        switch(phase)
        {
            case UIEvent::PHASE_BEGAN:
            case UIEvent::PHASE_DRAG:
            case UIEvent::PHASE_ENDED:
            {
                currentTool->height = GetDropperHeight();
                break;
            }
                
            default:
                break;
        }
    }
    else 
    {
        switch(phase)
        {
            case UIEvent::PHASE_BEGAN:
            {
                if(currentTool->absoluteDropperDrawing)
                {
                    currentTool->height = GetDropperHeight();
                }
                
				bool skipUndoPointCreation = false;
                if(LandscapeTool::TOOL_COPYPASTE == currentTool->type)
                {
					//CopyPasteBegin returns true when copyFrom point is set
					//no need to create undo point in this case
					skipUndoPointCreation = CopyPasteBegin();
					
					if (!skipUndoPointCreation)
					{
						DVASSERT(oldTilemap == NULL);
						workingLandscape->UpdateFullTiledTexture();
						oldTilemap = tilemaskTexture->CreateImageFromMemory();
					}
                }

				if (!skipUndoPointCreation)
				{
					StoreOriginalHeightmap();
				}

                editingIsEnabled = true;
                UpdateToolImage();

                break;
            }
                
            case UIEvent::PHASE_DRAG:
            {
                if(editingIsEnabled && !intersects)
                {
                    editingIsEnabled = false;

					CreateUndoPoint();
                }
                else if(!editingIsEnabled && intersects)
                {
                    editingIsEnabled = true;
                    UpdateToolImage();

					StoreOriginalHeightmap();

					if(LandscapeTool::TOOL_COPYPASTE == currentTool->type)
					{
						DVASSERT(oldTilemap == NULL);
						workingLandscape->UpdateFullTiledTexture();
						oldTilemap = tilemaskTexture->CreateImageFromMemory();
					}
                }
                break;
            }
                
            case UIEvent::PHASE_ENDED:
            {
                editingIsEnabled = false;

				CreateUndoPoint();

                break;
            }
                
            default:
                break;
        }
    }
}

Heightmap* LandscapeEditorHeightmap::GetHeightmap()
{
	return landscapesController->GetCurrentHeightmap();
}

void LandscapeEditorHeightmap::UpdateHeightmap(Heightmap* heightmap)
{
	UpdateHeightmap(Rect(0, 0, (float32)heightmap->Size()-1.f, (float32)heightmap->Size()-1.f));
}

bool LandscapeEditorHeightmap::CopyPasteBegin()
{
    bool start = IsKeyModificatorPressed(DVKEY_ALT);
    if(start)
    {
        copyFromCenter = landscapePoint;
        copyToCenter = Vector2(-1.0f, -1.0f);
    }
    else if(Vector2(-1.0f, -1.0f) == copyToCenter)
    {
        copyToCenter = landscapePoint;
    }

	return start;
}

void LandscapeEditorHeightmap::HideAction()
{
    landscapesController->CursorDisable();
    
    SafeRelease(toolImage);

    landscapesController->ReleaseEditorLandscape();
    SafeRelease(landscapesController);

	if (tilemaskWasChanged)
	{
		tilemaskWasChanged = false;
		workingLandscape->UpdateFullTiledTexture();
		Image* image = tilemaskTexture->CreateImageFromMemory();
		ImageLoader::Save(image, TextureDescriptor::GetPathnameForFormat(tilemaskPathname, PNG_FILE));
		SafeRelease(image);
	}

    SafeRelease(tilemaskImage);
    SafeRelease(toolImageTile);

    // RETURN TO THIS CODE LATER
    // workingLandscape->SetDebugFlags(workingLandscape->GetDebugFlags() & ~DebugRenderComponent::DEBUG_DRAW_GRID);
    workingLandscape->BuildLandscapeFromHeightmapImage(savedPath, workingLandscape->GetBoundingBox());
    workingLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, tilemaskPathname);

	Texture* texture = workingLandscape->GetTexture(Landscape::TEXTURE_TILE_MASK);
	texture->Reload();
}

void LandscapeEditorHeightmap::ShowAction()
{
    prevToolSize = 0.f;
    
    SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
    landscapesController = activeScene->GetLandscapesController();
    
    landscapesController->CreateEditorLandscape();    
    
    SafeRetain(landscapesController);
    
    savedPath = workingLandscape->GetHeightmapPathname();

    Heightmap *heightmap = landscapesController->GetCurrentHeightmap();
    landscapeSize = heightmap->Size();

    landscapesController->CursorEnable();
    
    CreateTilemaskImage();
}

void LandscapeEditorHeightmap::CreateTilemaskImage()
{
    SafeRelease(tilemaskImage);
    SafeRelease(toolImageTile);
    SafeRelease(tilemaskTexture);

    tilemaskWasChanged = false;
    prevToolSize = -1;
    
    copyFromCenter = Vector2(-1.0f, -1.0f);
    copyToCenter = Vector2(-1.0f, -1.0f);
    
    Texture *mask = workingLandscape->GetTexture(Landscape::TEXTURE_TILE_MASK);
    if(mask)
    {
        tilemaskPathname = mask->GetPathname();
        tilemaskImage = CreateTopLevelImage(TextureDescriptor::GetPathnameForFormat(tilemaskPathname, (ImageFileFormat)EditorSettings::Instance()->GetTextureViewFileFormat()));
        tilemaskTexture = Texture::CreateFromData(tilemaskImage->format, tilemaskImage->GetData(), tilemaskImage->GetWidth(), tilemaskImage->GetHeight(), false);
    }
    
    workingLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, tilemaskTexture);
    workingLandscape->UpdateFullTiledTexture();
}

void LandscapeEditorHeightmap::SaveTextureAction(const String &pathToFile)
{
    Heightmap *heightmap = landscapesController->GetCurrentHeightmap();
    if(heightmap)
    {
        String heightmapPath = pathToFile;
        String extension = FileSystem::Instance()->GetExtension(pathToFile);
        if(Heightmap::FileExtension() != extension)
        {
            heightmapPath = FileSystem::Instance()->ReplaceExtension(heightmapPath, Heightmap::FileExtension());
        }
        savedPath = heightmapPath;
        heightmap->Save(heightmapPath);
    }
}

NodesPropertyControl *LandscapeEditorHeightmap::GetPropertyControl(const Rect &rect)
{
    LandscapeEditorPropertyControl *propsControl = 
		(LandscapeEditorPropertyControl *)PropertyControlCreator::Instance()->CreateControlForLandscapeEditor(workingScene, rect, LandscapeEditorPropertyControl::HEIGHT_EDITOR_MODE);
    
    propsControl->SetDelegate(this);
    
    return propsControl;
}


void LandscapeEditorHeightmap::LandscapeEditorSettingsChanged(LandscapeEditorSettings *)
{
}

void LandscapeEditorHeightmap::TextureWillChanged(const String &forKey)
{
    if("property.landscape.texture.heightmap" == forKey)
    {
        if(savedPath.length())
        {
            SaveTextureAction(savedPath);

			StoreOriginalHeightmap();
        }
    }
    else if("property.landscape.texture.tilemask" == forKey) 
    {
        if(tilemaskImage && tilemaskWasChanged)
        {
            tilemaskWasChanged = false;
            ImageLoader::Save(tilemaskImage, tilemaskPathname);
        }
    }
}

void LandscapeEditorHeightmap::TextureDidChanged(const String &forKey)
{
    if("property.landscape.texture.heightmap" == forKey)
    {
        savedPath = workingLandscape->GetHeightmapPathname();
        
        Heightmap *heightmap = landscapesController->GetCurrentHeightmap();
        landscapeSize = heightmap->Size();

		CreateHeightmapUndo();
    }
    else if("property.landscape.texture.tilemask" == forKey)
    {
        CreateTilemaskImage();
    }
}

void LandscapeEditorHeightmap::OnToolSelected(LandscapeTool *newTool)
{
    LandscapeEditorBase::OnToolSelected(newTool);
    prevToolSize = -1.0f;
    
    copyFromCenter = Vector2(-1.0f, -1.0f);
    copyToCenter = Vector2(-1.0f, -1.0f);
    
    SafeRelease(toolImage);
}

void LandscapeEditorHeightmap::OnShowGrid(bool show)
{
    LandscapeEditorBase::OnShowGrid(show);
    
    if(landscapesController)
    {
        Landscape *landscape = landscapesController->GetCurrentLandscape();
        // RETURN TO THIS CODE LATER
        //landscape->SetDebugFlags(workingLandscape->GetDebugFlags());
    }
}


void LandscapeEditorHeightmap::UpdateHeightmap(const Rect &updatedRect)
{
    Heightmap *heightmap = landscapesController->GetCurrentHeightmap();

    Rect clippedRect;
    clippedRect.x = (float32)Clamp((int32)updatedRect.x, 0, heightmap->Size()-1);
    clippedRect.y = (float32)Clamp((int32)updatedRect.y, 0, heightmap->Size()-1);
  
    clippedRect.dx = Clamp((updatedRect.x + updatedRect.dx), 0.f, (float32)heightmap->Size() - 1.f) - clippedRect.x;
    clippedRect.dy = Clamp((updatedRect.y + updatedRect.dy), 0.f, (float32)heightmap->Size() - 1.f) - clippedRect.y;
    
    
    if(heightmapNode)
    {
        heightmapNode->UpdateHeightmapRect(clippedRect);
    }
    
    landscapesController->HeghtWasChanged(clippedRect);
}

void LandscapeEditorHeightmap::RecreateHeightmapNode()
{
    SafeRelease(heightmapNode);
    
    Landscape *landscape = landscapesController->GetCurrentLandscape();
    heightmapNode = new HeightmapNode(workingScene, landscape);
    
    workingScene->AddNode(heightmapNode);
}

void LandscapeEditorHeightmap::CreateUndoPoint()
{
	if (LandscapeTool::TOOL_COPYPASTE == currentTool->type)
		CreateCopyPasteUndo();
	else
		CreateHeightmapUndo();
}

void LandscapeEditorHeightmap::CreateHeightmapUndo()
{
	if (oldHeightmap)
	{
		CommandsManager::Instance()->ExecuteAndRelease(new CommandDrawHeightmap(oldHeightmap, GetHeightmap()));
		SafeRelease(oldHeightmap);
	}
}

void LandscapeEditorHeightmap::CreateCopyPasteUndo()
{
	if (oldHeightmap && oldTilemap)
	{
		workingLandscape->UpdateFullTiledTexture();
		Texture* texture = tilemaskTexture;
		Image* image = texture->CreateImageFromMemory();

		CommandsManager::Instance()->ExecuteAndRelease(new CommandCopyPasteHeightmap(currentTool->copyHeightmap,
																					 currentTool->copyTilemask,
																					 oldHeightmap,
																					 GetHeightmap(),
																					 oldTilemap,
																					 image,
																					 tilemaskPathname));
		SafeRelease(oldHeightmap);
		SafeRelease(oldTilemap);
		SafeRelease(image);
	}
}

void LandscapeEditorHeightmap::UpdateLandscapeTilemap(Texture* texture)
{
	SafeRelease(tilemaskTexture);
	SafeRelease(tilemaskImage);

	workingLandscape->UpdateFullTiledTexture();
	tilemaskTexture = SafeRetain(texture);
	tilemaskImage = tilemaskTexture->CreateImageFromMemory();

	workingLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, tilemaskTexture);
	workingLandscape->UpdateFullTiledTexture();

	tilemaskWasChanged = true;
}

void LandscapeEditorHeightmap::StoreOriginalHeightmap()
{
	DVASSERT(oldHeightmap == NULL);
	oldHeightmap = GetHeightmap()->Clone(oldHeightmap);
}
