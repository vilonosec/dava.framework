    //
//  ButtonNodeMetadata.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#include "UIButtonMetadata.h"
#include "EditorFontManager.h"
#include "UIControlStateHelper.h"

#include "PropertyNames.h"
#include "StringUtils.h"
#include "StringConstants.h"

using namespace DAVA;

UIButtonMetadata::UIButtonMetadata(QObject* parent) :
    UITextControlMetadata(parent)
{
}

UIButton* UIButtonMetadata::GetActiveUIButton() const
{
    return dynamic_cast<UIButton*>(GetActiveUIControl());
}

void UIButtonMetadata::SetLocalizedTextKey(const QString& value)
{
    if (!VerifyActiveParamID() || !this->GetActiveTreeNode())
    {
        return;
    }

    // Update the control with the value.
	WideString localizationValue = LocalizationSystem::Instance()->GetLocalizedString(QStrint2WideString(value));
    HierarchyTreeNode* node = this->GetActiveTreeNode();

	for(uint32 i = 0; i < GetStatesCount(); ++i)
	{
		// Update both key and value for all the states requested.
		node->GetExtraData().SetLocalizationKey(QStrint2WideString(value), this->uiControlStates[i]);
		GetActiveUIButton()->SetStateText(this->uiControlStates[i], localizationValue);
	}

    UpdatePropertyDirtyFlagForLocalizedText();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForLocalizedText()
{
    // Compare all the states with reference one.
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        if (curState == GetReferenceState())
        {
            continue;
        }
            
        bool curStateDirty = (GetLocalizedTextKeyForState(curState) !=
                              GetLocalizedTextKeyForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::LOCALIZED_TEXT_KEY_PROPERTY_NAME, curStateDirty);
    }
}

Font * UIButtonMetadata::GetFont()
{
    if (VerifyActiveParamID())
    {
        return GetFontForState(this->uiControlStates[GetActiveStateIndex()]);
    }
    return EditorFontManager::Instance()->GetDefaultFont();
}

void UIButtonMetadata::SetFont(Font * font)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    if (font)
    {
        font->SetSize(GetFontSize());
		for (uint32 i = 0; i < this->GetStatesCount(); ++i)
		{
			GetActiveUIButton()->SetStateFont(this->uiControlStates[i], font);
		}
        UpdatePropertyDirtyFlagForFont();
    }
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForFont()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

        bool curStateDirty = (GetFontForState(curState) !=
                              GetFontForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::FONT_PROPERTY_NAME, curStateDirty);
    }
}

Font * UIButtonMetadata::GetFontForState(UIControl::eControlState state) const
{
    UIStaticText *buttonText = GetActiveUIButton()->GetStateTextControl(state);
    if (buttonText)
    {
        return buttonText->GetFont();
    }
    return EditorFontManager::Instance()->GetDefaultFont();
}

float UIButtonMetadata::GetFontSize() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetFontSizeForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetFontSize(float fontSize)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		UIStaticText *buttonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[i]);
		if (!buttonText)
		{
			return;
		}
    
		Font *font = buttonText->GetFont();
		if (!font)
		{
			return;
		}
        
		font->SetSize(fontSize);
		buttonText->SetFont(font);
	}

    UpdatePropertyDirtyFlagForFontSize();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForFontSize()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

        bool curStateDirty = (GetFontSizeForState(curState) !=
                              GetFontSizeForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::FONT_SIZE_PROPERTY_NAME, curStateDirty);
    }
}

float UIButtonMetadata::GetFontSizeForState(UIControl::eControlState state) const
{
   UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(state);
   if (referenceButtonText)
    {
        Font* referenceFont = referenceButtonText->GetFont();
        if (referenceFont)
        {
            return referenceFont->GetSize();
        }
    }
    
    return -1.0f;
}

QColor UIButtonMetadata::GetFontColor() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetFontColorForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetFontColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetActiveUIButton()->SetStateFontColor(this->uiControlStates[i], QTColorToDAVAColor(value));
	}
    UpdatePropertyDirtyFlagForFontColor();
}

float UIButtonMetadata::GetShadowOffsetX() const
{
    if (VerifyActiveParamID())
    {
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[GetActiveStateIndex()]);
    	if (referenceButtonText)
    	{
			return referenceButtonText->GetShadowOffset().x;
    	}
	}
    
	return -1.0f;	
}

void UIButtonMetadata::SetShadowOffsetX(float offset)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[i]);
		if (referenceButtonText)
		{
			Vector2 shadowOffset = GetOffsetX(referenceButtonText->GetShadowOffset(), offset);
			referenceButtonText->SetShadowOffset(shadowOffset);
		}
	}
}
	
float UIButtonMetadata::GetShadowOffsetY() const
{
    if (VerifyActiveParamID())
    {
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[GetActiveStateIndex()]);
    	if (referenceButtonText)
    	{
			return referenceButtonText->GetShadowOffset().y;
    	}
	}
    
	return -1.0f;	
}

void UIButtonMetadata::SetShadowOffsetY(float offset)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[i]);
		if (referenceButtonText)
		{
			Vector2 shadowOffset = GetOffsetY(referenceButtonText->GetShadowOffset(), offset);
			referenceButtonText->SetShadowOffset(shadowOffset);
		}
	}
}
	
QColor UIButtonMetadata::GetShadowColor() const
{
    if (VerifyActiveParamID())
    {
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[GetActiveStateIndex()]);
    	if (referenceButtonText)
    	{
			return DAVAColorToQTColor(referenceButtonText->GetShadowColor());
    	}
	}
    
	return QColor();
}

void UIButtonMetadata::SetShadowColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[i]);
		if (referenceButtonText)
		{
			referenceButtonText->SetShadowColor(QTColorToDAVAColor(value));
		}
	}
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForFontColor()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetFontColorForState(curState) !=
                              GetFontColorForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::FONT_COLOR_PROPERTY_NAME, curStateDirty);
    }
}

QColor UIButtonMetadata::GetFontColorForState(UIControl::eControlState state) const
{
    UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(state);
    if (referenceButtonText)
    {
		return DAVAColorToQTColor(referenceButtonText->GetTextColor());
    }
    
    return QColor();
}

void UIButtonMetadata::SetSprite(const QString& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		//If empty string value is used - remove sprite
		if (value.isEmpty())
		{
			GetActiveUIButton()->SetStateSprite(this->uiControlStates[i], NULL);
		}
		else
		{
			GetActiveUIButton()->SetStateSprite(this->uiControlStates[i],
												TruncateTxtFileExtension(value).toStdString());
		}
	}

    UpdatePropertyDirtyFlagForSpriteName();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForSpriteName()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
            
        bool curStateDirty = (GetSpriteNameForState(curState) !=
                              GetSpriteNameForState(this->GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::SPRITE_PROPERTY_NAME, curStateDirty);
    }
}

QString UIButtonMetadata::GetSpriteNameForState(UIControl::eControlState state) const
{
    Sprite* sprite = GetActiveUIButton()->GetStateSprite(state);
    if (sprite == NULL)
    {
        return StringConstants::NO_SPRITE_IS_SET;
    }

    return sprite->GetRelativePathname().c_str();
}

QString UIButtonMetadata::GetSprite()
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }
    
    return GetSpriteNameForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetSpriteFrame(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		Sprite* sprite = GetActiveUIButton()->GetStateSprite(this->uiControlStates[i]);
		if (sprite == NULL)
		{
			continue;
		}
		
		if (sprite->GetFrameCount() <= value)
		{
			// No way to set this frame.
			continue;
		}
		
		GetActiveUIButton()->SetStateFrame(this->uiControlStates[i], value);
	}

    UpdatePropertyDirtyFlagForSpriteFrame();
}

int UIButtonMetadata::GetSpriteFrame()
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }

    return GetSpriteFrameForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForSpriteFrame()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

        bool curStateDirty = (GetSpriteFrameForState(curState) !=
                              GetSpriteFrameForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::SPRITE_FRAME_PROPERTY_NAME, curStateDirty);
    }
}

int UIButtonMetadata::GetSpriteFrameForState(UIControl::eControlState state) const
{
    return GetActiveUIButton()->GetStateFrame(state);
}

UIControl::eControlState UIButtonMetadata::GetCurrentStateForLocalizedText() const
{
    // UIButton is state-aware, so return current state.
    return this->uiControlStates[GetActiveStateIndex()];
}

QColor UIButtonMetadata::GetColor()
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
    
    return GetColorForState(this->uiControlStates[GetActiveStateIndex()]);
}


void UIButtonMetadata::SetColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		// Yuri Coder, 08/11/2012. According to Dizzer's request update the button background
		// before assigning color - otherwise it might be assigned to the default background.
		GetActiveUIButton()->CreateBackgroundForState(this->uiControlStates[i]);

		UIControlBackground* background = GetActiveUIButton()->GetStateBackground(this->uiControlStates[i]);
		if (!background)
		{
			continue;
		}

		background->color = QTColorToDAVAColor(value);
	}
    UpdatePropertyDirtyFlagForColor();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForColor()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetColorForState(curState) !=
                              GetColorForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::BACKGROUND_COLOR_PROPERTY_NAME, curStateDirty);
    }
}

QColor UIButtonMetadata::GetColorForState(UIControl::eControlState state) const
{
    UIControlBackground* background = GetActiveUIButton()->GetStateBackground(state);
    if (background)
    {
        return DAVAColorToQTColor(background->color);
    }
    
    return QColor();
}

int UIButtonMetadata::GetDrawType()
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::DRAW_ALIGNED;
    }
    
    return GetActiveUIButton()->GetStateDrawType(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetDrawType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetActiveUIButton()->SetStateDrawType(this->uiControlStates[i], (UIControlBackground::eDrawType)value);
	}
    UpdatePropertyDirtyFlagForDrawType();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForDrawType()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

        bool curStateDirty = (GetActiveUIButton()->GetStateDrawType(curState) !=
                              GetActiveUIButton()->GetStateDrawType(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::DRAW_TYPE_PROPERTY_NAME, curStateDirty);
    }
}

int UIButtonMetadata::GetColorInheritType()
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    }

    return GetColorInheritTypeForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetColorInheritType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		UIControlBackground* background = GetActiveUIButton()->GetStateBackground(this->uiControlStates[i]);
		if (!background)
		{
			continue;
		}

		background->SetColorInheritType((UIControlBackground::eColorInheritType)value);
	}
    UpdatePropertyDirtyFlagForColorInheritType();
}

int UIButtonMetadata::GetColorInheritTypeForState(UIControl::eControlState state) const
{
    UIControlBackground* background = GetActiveUIButton()->GetStateBackground(state);
    if (!background)
    {
        return UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    }
    
    return background->GetColorInheritType();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForColorInheritType()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetColorInheritTypeForState(curState) !=
                              GetColorInheritTypeForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::COLOR_INHERIT_TYPE_PROPERTY_NAME, curStateDirty);
    }
}

int UIButtonMetadata::GetAlign()
{
    if (!VerifyActiveParamID())
    {
        return ALIGN_TOP;
    }
    
    return GetAlignForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetActiveUIButton()->SetStateAlign(this->uiControlStates[i], value);
	}
    UpdatePropertyDirtyFlagForAlign();
}

int UIButtonMetadata::GetAlignForState(UIControl::eControlState state) const
{
	return GetActiveUIButton()->GetStateAlign(state);
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForAlign()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetAlignForState(curState) !=
                              GetAlignForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::ALIGN_PROPERTY_NAME, curStateDirty);
    }
}


int UIButtonMetadata::GetSpriteModification()
{
	if (!VerifyActiveParamID())
	{
		return UIControlBackground::DRAW_ALIGNED;
	}

	return GetSpriteModificationForState(uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetSpriteModification(int value)
{
	if (!VerifyActiveParamID())
	{
		return;
	}

	for (uint32 i = 0; i < GetStatesCount(); ++i)
	{
		UIControlBackground* background = GetActiveUIButton()->GetStateBackground(uiControlStates[i]);
		if (!background)
		{
			continue;
		}

		background->SetModification(value);
	}
	UpdatePropertyDirtyFlagForSpriteModification();
}

int UIButtonMetadata::GetSpriteModificationForState(UIControl::eControlState state) const
{
	UIControlBackground* background = GetActiveUIButton()->GetStateBackground(state);

	if (!background)
	{
		return UIControlBackground::DRAW_ALIGNED;
	}

	return background->GetModification();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForSpriteModification()
{
	int statesCount = UIControlStateHelper::GetUIControlStatesCount();
	for (int i = 0; i < statesCount; i ++)
	{
		UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

		bool curStateDirty = (GetSpriteModificationForState(curState) !=
							  GetSpriteModificationForState(GetReferenceState()));
		SetStateDirtyForProperty(curState, PropertyNames::SPRITE_MODIFICATION_PROPERTY_NAME, curStateDirty);
	}
}


// Initialize the control(s) attached.
void UIButtonMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    BaseMetadata::InitializeControl(controlName, position);

    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UIButton* button = dynamic_cast<UIButton*>(this->treeNodeParams[i].GetUIControl());

        WideString controlText = StringToWString(button->GetName());
        HierarchyTreeNode* activeNode = GetTreeNode(i);
    
        // Initialize the button for all states.
        int statesCount = UIControlStateHelper::GetUIControlStatesCount();
        for (int stateID = 0; stateID < statesCount; stateID ++)
        {
            UIControl::eControlState state = UIControlStateHelper::GetUIControlState(stateID);
            button->SetStateFont(state, EditorFontManager::Instance()->GetDefaultFont());
            button->SetStateText(state, controlText);

            // Button is state-aware.
            activeNode->GetExtraData().SetLocalizationKey(controlText, state);
        }
        
        // Define some properties for the reference state.
        button->SetStateDrawType(GetReferenceState(), UIControlBackground::DRAW_SCALE_TO_RECT);
    }
}

void UIButtonMetadata::UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    UIButton* button = GetActiveUIButton();

    // Button is state-aware.
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int stateID = 0; stateID < statesCount; stateID ++)
    {
        UIControl::eControlState state = UIControlStateHelper::GetUIControlState(stateID);
        UIStaticText* textControl = button->GetStateTextControl(state);
        if (!textControl)
        {
            continue;
        }
        
        UpdateStaticTextExtraData(textControl, state, extraData, updateStyle);
    }
    
    if (updateStyle == UPDATE_EXTRADATA_FROM_CONTROL)
    {
        // Also need to recover Dirty Flags in this case.
        RecoverPropertyDirtyFlags();
    }
}

void UIButtonMetadata::RecoverPropertyDirtyFlags()
{
    UpdatePropertyDirtyFlagForLocalizedText();
    UpdatePropertyDirtyFlagForFont();
    UpdatePropertyDirtyFlagForFontSize();
    UpdatePropertyDirtyFlagForColor();

    UpdatePropertyDirtyFlagForSpriteName();
    UpdatePropertyDirtyFlagForSpriteFrame();
    
    UpdatePropertyDirtyFlagForDrawType();
    UpdatePropertyDirtyFlagForColorInheritType();
    UpdatePropertyDirtyFlagForAlign();
}
