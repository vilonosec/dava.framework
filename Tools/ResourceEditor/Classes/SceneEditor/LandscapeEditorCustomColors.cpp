#include "LandscapeEditorCustomColors.h"

#include "LandscapeTool.h"
#include "LandscapeToolsPanelCustomColors.h"
#include "PropertyControlCreator.h"
#include "EditorScene.h"
#include "EditorConfig.h"

#include "HeightmapNode.h"

#include "../LandscapeEditor/EditorHeightmap.h"
#include "../LandscapeEditor/EditorLandscape.h"
#include "EditorBodyControl.h"
#include "SceneGraph.h"
#include "../Qt/Main/QtMainWindowHandler.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Main/QtUtils.h"
#include "../Commands/CustomColorCommands.h"
#include "../Commands/CommandsManager.h"

#define CUSTOM_COLOR_TEXTURE_PROP "customColorTexture"

LandscapeEditorCustomColors::LandscapeEditorCustomColors(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl, const Rect &toolsRect)
    :   LandscapeEditorBase(newDelegate, parentControl)
{
	wasTileMaskToolUpdate = false;

    settings = NULL;
    
    toolsPanel = new LandscapeToolsPanelCustomColors(this, toolsRect);

    editingIsEnabled = false;

	paintColor = Color(1.f, 1.f, 1.f, 1.0f);

	colorSprite = NULL;
	texSurf = NULL;
	circleTexture = NULL;

	isFogEnabled = false;
	isCursorTransparent = false;

	radius = 64;
	UpdateCircleTexture(false);

	originalTexture = NULL;

	unsavedChanges = false;
}

LandscapeEditorCustomColors::~LandscapeEditorCustomColors()
{
	SafeRelease(texSurf);
	SafeRelease(colorSprite);
	SafeRelease(originalTexture);
}


void LandscapeEditorCustomColors::Draw(const DAVA::UIGeometricData &geometricData)
{
    if(wasTileMaskToolUpdate)
	{
		PrepareRenderLayers();
		PerformLandscapeDraw();
		wasTileMaskToolUpdate = false;
	}
}

void LandscapeEditorCustomColors::PrepareRenderLayers()
{
	UpdateCircleTexture(false);

    Sprite* blankSprite = Sprite::CreateFromTexture(circleTexture, 0, 0, (float32)circleTexture->width, (float32)circleTexture->height);
    
	//fill color sprite to get opportunity to save its texture separately 
	RenderManager::Instance()->SetRenderTarget(colorSprite);
	
	Vector2 newPoint = workingLandscape->GetCursor()->GetCursorPosition(); 
	RenderManager::Instance()->SetColor(paintColor);
	
    blankSprite->SetPosition(newPoint); 
	blankSprite->Draw();
	RenderManager::Instance()->RestoreRenderTarget();
    RenderManager::Instance()->ResetColor();
	SafeRelease(blankSprite);
	
}

void LandscapeEditorCustomColors::PerformLandscapeDraw()
{
	Sprite* sprLandscape = Sprite::CreateFromTexture(texSurf, 0, 0, texSurf->GetWidth(), texSurf->GetHeight());
	Sprite* sprTargetSurf = Sprite::CreateAsRenderTarget(texSurf->width, texSurf->height, FORMAT_RGBA8888);
	//render original and color layer to final container 
    RenderManager::Instance()->SetRenderTarget(sprTargetSurf);
	sprLandscape->Draw();
	RenderManager::Instance()->SetColor(1.f, 1.f, 1.f, .5f);
    colorSprite->Draw();
	RenderManager::Instance()->SetColor(Color::White());
    texSurf->GenerateMipmaps();
    RenderManager::Instance()->RestoreRenderTarget();

	workingLandscape->SetTexture(Landscape::TEXTURE_TILE_FULL, sprTargetSurf->GetTexture());
	
	SafeRelease(sprTargetSurf);
	SafeRelease(sprLandscape);
	
}

//Bresenham's algorithm
void LandscapeEditorCustomColors::DrawCircle(Vector<Vector<bool> >& matrixForCircle)
{
	const int matrSize = matrixForCircle.size();
	int x0 = matrSize/2 - 1;
	int y0 = matrSize/2 - 1 ;
	int radius = matrSize/2 - 2 ;

	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;
	
	matrixForCircle[x0][ y0 + radius] = true;
	matrixForCircle[x0][ y0 - radius] = true;
	matrixForCircle[x0 + radius][ y0] = true;
	matrixForCircle[x0 - radius][ y0] =	true;
	
	while(x < y)
	{
		if(f >= 0) 
		{
		  y--;
		  ddF_y += 2;
		  f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;    
		matrixForCircle[x0 + x][ y0 + y] = true;
		matrixForCircle[x0 - x][ y0 + y] = true;
		matrixForCircle[x0 + x][ y0 - y] = true;
		matrixForCircle[x0 - x][ y0 - y] = true;
		matrixForCircle[x0 + y][ y0 + x] = true;
		matrixForCircle[x0 - y][ y0 + x] = true;
		matrixForCircle[x0 + y][ y0 - x] = true;
		matrixForCircle[x0 - y][ y0 - x] = true;
	}
	for (int i = 0; i< matrSize; ++i)
	{
		int startOfBlackBlock = 0;
		int endOfBlackBlock = matrSize - 1;
		for (int j = startOfBlackBlock; j < matrSize; ++j)
		{
			if(matrixForCircle[i][j])
			{
				startOfBlackBlock = j;
				break;
			}
			startOfBlackBlock = endOfBlackBlock;
		}

		for (int j = endOfBlackBlock; j > startOfBlackBlock; --j)
		{
			if(matrixForCircle[i][j])
			{
				endOfBlackBlock = j;
				break;
			}
		}

		for (int j = startOfBlackBlock; j < endOfBlackBlock; ++j)
		{
			matrixForCircle[i][j] = true;
		}
	}
}

uint8*	LandscapeEditorCustomColors::DrawFilledCircleWithFormat(uint32 radius, DAVA::PixelFormat format, bool setTransparent)
{
	if(FORMAT_RGBA8888 != format || radius == 0)
		return NULL;

    uint32 texFormatSize = 4;
    uint32 size = 4 * radius * radius * texFormatSize;
    uint8* texArr = new uint8[size];
	
    memset(texArr, 0, size);

	Vector<Vector<bool> > matrixForCircle;
	
	for (uint32 i = 0; i < radius*2; ++i)
	{
		Vector<bool> tmp;
		tmp.resize(radius*2);
		matrixForCircle.push_back(tmp);
	}
	DrawCircle(matrixForCircle);
	for(uint32 i = 0; i <matrixForCircle.size(); ++i)
	{
		for(uint32 j = 0; j < matrixForCircle.size(); ++j)
		{
			if(matrixForCircle[i][j])
			{
				uint32 lineNumber = i * matrixForCircle.size() + j;
				uint32 blockOffset = lineNumber*4;
				texArr[blockOffset]		= 0xff;
				texArr[blockOffset + 1] = 0xff;
				texArr[blockOffset + 2] = 0xff;
				if(setTransparent)
				{
					texArr[blockOffset + 3] = 0x00;
				}
				else
				{
					texArr[blockOffset + 3] = 0xff;
				}
				
				
			}
		}
	}

	isCursorTransparent = setTransparent;

	return texArr;
}

void LandscapeEditorCustomColors::UpdateCursor()
{
	if(currentTool && currentTool->sprite && currentTool->size)
	{
		Vector2 pos = landscapePoint - Vector2(radius, radius)/2;
		UpdateCircleTexture(false);
		workingLandscape->SetCursorTexture(circleTexture);
		workingLandscape->SetBigTextureSize((float32)workingLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL)->GetWidth());
		workingLandscape->SetCursorPosition(pos);
		
		workingLandscape->SetCursorScale(radius*2);
	}
}

void LandscapeEditorCustomColors::SetRadius(int _radius)
{
	radius = _radius % 2 == 0 ? _radius : ++_radius;//in order to avoid dark boundaries
	UpdateCircleTexture(true);
}

void LandscapeEditorCustomColors::SetColor(const Color &newColor)
{
	paintColor = newColor;
}

void LandscapeEditorCustomColors::SaveColorLayer(const String &pathName)
{
	SaveTextureAction(pathName);
}

void LandscapeEditorCustomColors::LoadColorLayer(const String &pathName)
{
	LoadTextureAction(pathName);
}

void LandscapeEditorCustomColors::UpdateCircleTexture(bool setTransparent)
{
	if(isCursorTransparent == setTransparent)
	{
		if(NULL != circleTexture  )
		{
			return;
		}
	}
	uint8* texArr = DrawFilledCircleWithFormat(radius, FORMAT_RGBA8888, setTransparent);
	if(!texArr)
	{
		return;
	}
	SafeRelease(circleTexture);
	circleTexture = Texture::CreateFromData(FORMAT_RGBA8888, texArr, radius*2, radius*2,false);
	//check addref
	delete[] texArr;
}

void LandscapeEditorCustomColors::InputAction(int32 phase, bool intersects)
{
    switch(phase)
    {
        case UIEvent::PHASE_BEGAN:
        {
            editingIsEnabled = true;

			StoreOriginalState();

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

				StoreOriginalState();
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
    
    UpdateCircleTexture(false);
	wasTileMaskToolUpdate = true;
}

Image* LandscapeEditorCustomColors::StoreState()
{
	return colorSprite->GetTexture()->CreateImageFromMemory();
}

void LandscapeEditorCustomColors::RestoreState(DAVA::Image *image)
{
	Texture* texture = Texture::CreateTextFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight(), false);
	Sprite* sprite = Sprite::CreateFromTexture(texture, 0, 0, texture->GetWidth(), texture->GetHeight());

	uint32 width = 0;
	uint32 height = 0;

	if (colorSprite)
	{
		width = colorSprite->GetWidth();
		height = colorSprite->GetHeight();
	}
	else
	{
		width = texSurf->GetWidth();
		height = texSurf->GetHeight();
	}

	SafeRelease(colorSprite);
	colorSprite = Sprite::CreateAsRenderTarget(width, height, FORMAT_RGBA8888);

	RenderManager::Instance()->SetRenderTarget(colorSprite);
	sprite->Draw();
	RenderManager::Instance()->RestoreRenderTarget();

	SafeRelease(sprite);
	SafeRelease(texture);

	if (IsActive())
		PerformLandscapeDraw();

	unsavedChanges = true;
}

void LandscapeEditorCustomColors::HideAction()
{
	if(!IsActive())
	{
		return;
	}
	workingLandscape->CursorDisable();
	workingLandscape->SetFog(isFogEnabled);

	SafeRelease(texSurf);
	SafeRelease(circleTexture);
	
	QtMainWindowHandler::Instance()->SetCustomColorsWidgetsState(false);
}

void LandscapeEditorCustomColors::ShowAction()
{
    landscapeSize = GetLandscape()->GetTexture(Landscape::TEXTURE_TILE_FULL)->GetWidth();

	workingLandscape->CursorEnable();
	//save fog status and disable it for more convenience
	isFogEnabled = workingLandscape->IsFogEnabled();
	workingLandscape->SetFog(false);
	texSurf = SafeRetain( workingLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL));

	String loadFileName = GetCurrentSaveFileName();
	if(!loadFileName.empty())
		LoadTextureAction(loadFileName);

	if(NULL == colorSprite)
	{
		Texture* tex =  workingLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL);
		colorSprite = Sprite::CreateAsRenderTarget(tex->width, tex->height, FORMAT_RGBA8888);
		RenderManager::Instance()->SetRenderTarget(colorSprite);
		const Vector<Color> & colors = EditorConfig::Instance()->GetColorPropertyValues("LandscapeCustomColors");
		if(!colors.empty())
		{
			RenderManager::Instance()->ClearWithColor(colors[0].r, colors[0].g, colors[0].b, colors[0].a);
		}
		RenderManager::Instance()->RestoreRenderTarget();
	}

	PerformLandscapeDraw();

	QtMainWindowHandler::Instance()->SetCustomColorsWidgetsState(true);
}

void LandscapeEditorCustomColors::SaveTextureAction(const String &pathToFile)
{
	if(pathToFile.empty())
		return;

    if(colorSprite)
    {
		if(!pathToFile.empty())
		{
			Image *img = colorSprite->GetTexture()->CreateImageFromMemory();   
			if(img)
			{
				StoreSaveFileName(pathToFile);
				ImageLoader::Save(img, pathToFile);
				SafeRelease(img);

				unsavedChanges = false;
			}
		}
	}
}

void LandscapeEditorCustomColors::LoadTextureAction(const String &pathToFile)
{
	if(pathToFile.empty())
		return;

	Vector<Image*> images = ImageLoader::CreateFromFile(pathToFile);
	if(images.empty())
		return;

	Image* image = images.front();
	if(image)
	{
		Texture* texture = Texture::CreateFromData(image->GetPixelFormat(),
												   image->GetData(),
												   image->GetWidth(),
												   image->GetHeight(),
												   false);

		SafeRelease(colorSprite);
		colorSprite = Sprite::CreateAsRenderTarget(texSurf->GetWidth(), texSurf->GetHeight(), FORMAT_RGBA8888);
		Sprite* sprite = Sprite::CreateFromTexture(texture, 0, 0, texture->GetWidth(), texture->GetHeight());

		StoreOriginalState();

		RenderManager::Instance()->SetRenderTarget(colorSprite);
		sprite->Draw();
		RenderManager::Instance()->RestoreRenderTarget();
		PerformLandscapeDraw();

		SafeRelease(sprite);
		SafeRelease(texture);
		for_each(images.begin(), images.end(), SafeRelease<Image>);

		StoreSaveFileName(pathToFile);

		CreateUndoPoint();
	}
}

NodesPropertyControl *LandscapeEditorCustomColors::GetPropertyControl(const Rect &rect)
{
	LandscapeEditorPropertyControl *propsControl =
		(LandscapeEditorPropertyControl *)PropertyControlCreator::Instance()->CreateControlForLandscapeEditor(workingScene, rect, LandscapeEditorPropertyControl::COLORIZE_EDITOR_MODE);
		//(LandscapeEditorPropertyControl *)PropertyControlCreator::Instance()->CreateControlForLandscapeEditor(workingLandscape, rect, LandscapeEditorPropertyControl::COLORIZE_EDITOR_MODE);

	workingLandscape->SetTiledShaderMode(Landscape::TILED_MODE_TEXTURE);
	propsControl->SetDelegate(this);
	LandscapeEditorSettingsChanged(propsControl->Settings());

	return propsControl;
}

void LandscapeEditorCustomColors::LandscapeEditorSettingsChanged(LandscapeEditorSettings *newSettings)
{
    settings = newSettings;
}

void LandscapeEditorCustomColors::TextureWillChanged(const String &forKey)
{}

void LandscapeEditorCustomColors::TextureDidChanged(const String &forKey)
{}

void LandscapeEditorCustomColors::RecreateHeightmapNode()
{
    if(workingScene && heightmapNode)
    {
        workingScene->RemoveNode(heightmapNode);
    }
        
    SafeRelease(heightmapNode);
    heightmapNode = new HeightmapNode(workingScene, workingLandscape);
    workingScene->AddNode(heightmapNode);
}

bool LandscapeEditorCustomColors::SetScene(EditorScene *newScene)
{
    EditorLandscape *editorLandscape = dynamic_cast<EditorLandscape *>(newScene->GetLandscape(newScene));
    if(editorLandscape)
    {
        ShowErrorDialog(String("Cannot start color editor. Remove EditorLandscape from scene"));
        return false;
    }

    return LandscapeEditorBase::SetScene(newScene);
}

void LandscapeEditorCustomColors::StoreSaveFileName(const String& fileName)
{
	parent->GetSceneGraph()->UpdatePropertyPanel();
	if(NULL != workingLandscapeEntity)
	{
		KeyedArchive* customProps = workingLandscapeEntity->GetCustomProperties();
		customProps->SetString(CUSTOM_COLOR_TEXTURE_PROP, GetRelativePathToProjectPath(fileName));
	}
}

String LandscapeEditorCustomColors::GetCurrentSaveFileName()
{
	String currentSaveName = "";

	if(NULL != workingLandscapeEntity)
	{
		KeyedArchive* customProps = workingLandscapeEntity->GetCustomProperties();
		if(customProps->IsKeyExists(CUSTOM_COLOR_TEXTURE_PROP))
		{
			currentSaveName = customProps->GetString(CUSTOM_COLOR_TEXTURE_PROP);
		}
	}

	return GetAbsolutePathFromProjectPath(currentSaveName);
}

String LandscapeEditorCustomColors::GetScenePath()
{
	String sceneFilePath = SceneDataManager::Instance()->SceneGetActive()->GetScenePathname();
	String sceneFileName = "";
	FileSystem::Instance()->SplitPath(sceneFilePath, sceneFilePath, sceneFileName);

	return sceneFilePath;
}

String LandscapeEditorCustomColors::GetRelativePathToScenePath(const String &absolutePath)
{
	if(absolutePath.empty())
		return "";

	String relativePath = FileSystem::Instance()->AbsoluteToRelativePath(GetScenePath(), absolutePath);

	return relativePath;
}

String LandscapeEditorCustomColors::GetRelativePathToProjectPath(const String& absolutePath)
{
	if(absolutePath.empty())
		return "";

	String relativePath = FileSystem::Instance()->AbsoluteToRelativePath(EditorSettings::Instance()->GetProjectPath(), absolutePath);

	return relativePath;
}

String LandscapeEditorCustomColors::GetAbsolutePathFromScenePath(const String &relativePath)
{
	if(relativePath.empty())
		return "";

	String absolutePath = GetScenePath() + relativePath;
	absolutePath = FileSystem::Instance()->GetCanonicalPath(absolutePath);

	return absolutePath;
}


String LandscapeEditorCustomColors::GetAbsolutePathFromProjectPath(const String& relativePath)
{
	if(relativePath.empty())
		return "";

	String absolutePath = EditorSettings::Instance()->GetProjectPath() + relativePath;
	absolutePath = FileSystem::Instance()->GetCanonicalPath(absolutePath);

	return absolutePath;
}

void LandscapeEditorCustomColors::SaveTexture()
{
	if (unsavedChanges)
	{
		CommandsManager::Instance()->ExecuteAndRelease(new CommandSaveTextureCustomColors());
	}
	Close();
}

void LandscapeEditorCustomColors::ClearSceneResources()
{
	LandscapeEditorBase::ClearSceneResources();
	SafeRelease(colorSprite);
}

void LandscapeEditorCustomColors::UpdateLandscapeTilemap(Texture* texture)
{
	workingLandscape->SetTexture(Landscape::TEXTURE_TILE_FULL, texSurf);
	workingLandscape->SetTexture(Landscape::TEXTURE_TILE_MASK, texture);
	workingLandscape->UpdateFullTiledTexture();
	
	Image* image = workingLandscape->GetTexture(Landscape::TEXTURE_TILE_MASK)->CreateImageFromMemory();
	ImageLoader::Save(image, texture->GetPathname());
	SafeRelease(image);
	
	SafeRelease(texSurf);
	
	texSurf = SafeRetain(workingLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL));
	wasTileMaskToolUpdate = true;
}

void LandscapeEditorCustomColors::CreateUndoPoint()
{
	if (originalTexture)
	{
		Image* newTexture = StoreState();
		CommandsManager::Instance()->ExecuteAndRelease(new CommandDrawCustomColors(originalTexture, newTexture));
		SafeRelease(originalTexture);
		SafeRelease(newTexture);
	}
	unsavedChanges = true;
}

void LandscapeEditorCustomColors::StoreOriginalState()
{
	DVASSERT(originalTexture == NULL);
	originalTexture = StoreState();
}
