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
        * Created by Alexey 'Hottych' Prosin
=====================================================================================*/
#include "Utils/Utils.h"
#include "UI/UIStaticText.h"
#include "Render/RenderManager.h"
#include "Base/ObjectFactory.h"
#include "UI/UIYamlLoader.h"
#include "Utils/StringFormat.h"
#include "FileSystem/LocalizationSystem.h"
#include "Render/2D/FontManager.h"

namespace DAVA 
{
	
REGISTER_CLASS(UIStaticText);

UIStaticText::UIStaticText(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/) 
:	UIControl(rect, rectInAbsoluteCoordinates)
	, textColor(1.0f, 1.0f, 1.0f, 1.0f)
	, tempSize(0, 0)
	, shadowOffset(0, 0)
	, shadowColor(0, 0, 0, 1)
{
	inputEnabled = false;
	textBlock = TextBlock::Create(Vector2(rect.dx, rect.dy));
	background->SetAlign(ALIGN_TOP|ALIGN_LEFT);
	background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);

	shadowBg = new UIControlBackground();
}

UIStaticText::~UIStaticText()
{
	SafeRelease(textBlock);
	SafeRelease(shadowBg);
}

	
UIControl *UIStaticText::Clone()
{
	UIStaticText *t = new UIStaticText(GetRect());
	t->CopyDataFrom(this);
	return t;
}
	
void UIStaticText::CopyDataFrom(UIControl *srcControl)
{
	UIControl::CopyDataFrom(srcControl);
    UIStaticText *t = (UIStaticText *)srcControl;
    textBlock = t->textBlock->Clone();
    textColor = t->textColor;
    shadowColor = t->shadowColor;
    shadowOffset = t->shadowOffset;
    SafeRelease(shadowBg);
    shadowBg = t->shadowBg->Clone();
}
	
UIStaticText *UIStaticText::CloneStaticText()
{
	return (UIStaticText *)Clone();
}
	
void UIStaticText::SetText(const WideString& _string, const Vector2 &requestedTextRectSize/* = Vector2(0,0)*/)
{
	textBlock->SetRectSize(size);
	textBlock->SetText(_string, requestedTextRectSize);
	PrepareSprite();
}
	
void UIStaticText::SetFittingOption(int32 fittingType)
{
	textBlock->SetRectSize(size);
	textBlock->SetFittingOption(fittingType);
	PrepareSprite();
}

void UIStaticText::SetFont(Font * _font)
{
	textBlock->SetRectSize(size);
	textBlock->SetFont(_font);
	PrepareSprite();
}

void UIStaticText::SetTextColor(const Color& color)
{
	textColor = color;
}

void UIStaticText::SetShadowOffset(const Vector2 &offset)
{
	shadowOffset = offset;
}

void UIStaticText::SetShadowColor(const Color &color)
{
	shadowColor = color;
}
    
void UIStaticText::SetMultiline(bool _isMultilineEnabled, bool bySymbol)
{
	textBlock->SetRectSize(size);
	textBlock->SetMultiline(_isMultilineEnabled, bySymbol);
	PrepareSprite();
}
	
void UIStaticText::SetSpriteAlign(int32 align)
{
	SetAlign(align);
}

void UIStaticText::SetAlign(int32 _align)
{
	textBlock->SetAlign(_align);
}

int32 UIStaticText::GetAlign() const
{
	return textBlock->GetAlign();
}

const Vector2 &UIStaticText::GetTextSize()
{
	if (textBlock->IsSpriteReady())
	{
		return tempSize = textBlock->GetSprite()->GetSize();
	}
	else 
	{
		tempSize = Vector2(0,0);
	}

	return tempSize;
}

const Color &UIStaticText::GetTextColor() const
{
	return textColor;
}

const Color &UIStaticText::GetShadowColor() const
{
	return shadowColor;
}

const Vector2 &UIStaticText::GetShadowOffset() const
{
	return shadowOffset;
}

void UIStaticText::Draw(const UIGeometricData &geometricData)
{
	textBlock->SetRectSize(size);
	PrepareSprite();
	textBlock->PreDraw();

	if(0 != shadowColor.a && (0 != shadowOffset.dx || 0 != shadowOffset.dy))
	{
		UIGeometricData shadowGeomData = geometricData;

		shadowGeomData.position += shadowOffset;
		shadowGeomData.unrotatedRect += shadowOffset;

		shadowBg->SetAlign(background->GetAlign());
        shadowBg->SetPerPixelAccuracyType(background->GetPerPixelAccuracyType());
		shadowBg->SetDrawColor(shadowColor);
		shadowBg->Draw(shadowGeomData);
	}

	background->SetDrawColor(textColor);
	UIControl::Draw(geometricData);
}

void UIStaticText::SetFontColor(const Color& fontColor)
{
	SetTextColor(fontColor);
}
    
const Vector<WideString> & UIStaticText::GetMultilineStrings()
{
    return textBlock->GetMultilineStrings();
}
	
const WideString & UIStaticText::GetText()
{
	return textBlock->GetText();
}

void UIStaticText::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
	UIControl::LoadFromYamlNode(node, loader);
	
	YamlNode * fontNode = node->Get("font");
	YamlNode * textNode = node->Get("text");
	YamlNode * multilineNode = node->Get("multiline");
    YamlNode * multilineBySymbolNode = node->Get("multilineBySymbol");
    YamlNode * fittingNode = node->Get("fitting");
	YamlNode * textColorNode = node->Get("textcolor");
	YamlNode * shadowColorNode = node->Get("shadowcolor");
	YamlNode * shadowOffsetNode = node->Get("shadowoffset");

	if (fontNode)
	{
		const String & fontName = fontNode->AsString();
		Font * font = loader->GetFontByName(fontName);
		SetFont(font);
	}

	bool multiline = loader->GetBoolFromYamlNode(multilineNode, false);
    bool multilineBySymbol = loader->GetBoolFromYamlNode(multilineBySymbolNode, false);
	SetMultiline(multiline, multilineBySymbol);
	
    if(fittingNode)
    {
        int32 fittingArray[] = {TextBlock::FITTING_DISABLED, TextBlock::FITTING_ENLARGE, 
                                TextBlock::FITTING_REDUCE, TextBlock::FITTING_POINTS};
		String fittingValues[] = {"Disabled", "Enlarge", "Reduce", "Points"};

		const String & fittinOption = fittingNode->AsString();
        
        int32 fittingType = 0;
        for(int32 i = 0 ; i < 4; ++i)
        {
            size_t find = fittinOption.find(fittingValues[i]);
            if(find != fittinOption.npos)
            {
                fittingType |= fittingArray[i];
            }
        }

        SetFittingOption(fittingType);
    }
    
	if (textNode)
	{
		SetText(LocalizedString(textNode->AsWString()));
	}

	if(textColorNode)
	{
		Vector4 c = textColorNode->AsVector4();
		SetTextColor(Color(c.x, c.y, c.z, c.w));
	}

	if(shadowColorNode)
	{
		Vector4 c = shadowColorNode->AsVector4();
		SetShadowColor(Color(c.x, c.y, c.z, c.w));
	}

	if(shadowOffsetNode)
	{
		SetShadowOffset(shadowOffsetNode->AsVector2());
	}

	YamlNode * alignNode = node->Get("align");
	SetAlign(loader->GetAlignFromYamlNode(alignNode)); // NULL is also OK here.
}

YamlNode * UIStaticText::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);

    // Sprite node is not needed for UITextField.
    YamlNode *spriteNode = node->Get("sprite");
    if (spriteNode)
    {
        node->RemoveNodeFromMap("sprite");
    }
	
	UIStaticText *baseControl = new UIStaticText();	

    //Temp variable
    VariantType *nodeValue = new VariantType();
    
    //Control Type
	SetPreferredNodeType(node, "UIStaticText");

    //Font
    //Get font name and put it here
    nodeValue->SetString(FontManager::Instance()->GetFontName(this->GetFont()));
    node->Set("font", nodeValue);

	//TextColor
	nodeValue->SetVector4(Vector4(textColor.r, textColor.g, textColor.b, textColor.a));
	node->Set("textcolor", nodeValue);

	// ShadowColor
	nodeValue->SetVector4(Vector4(shadowColor.r, shadowColor.g, shadowColor.b, shadowColor.a));
	node->Set("shadowcolor", nodeValue);

	// ShadowOffset
	nodeValue->SetVector2(GetShadowOffset());
	node->Set("shadowoffset", nodeValue);

    //Text
    nodeValue->SetWideString(GetText());
    node->Set("text", nodeValue);    
    //Multiline
	if (baseControl->textBlock->GetMultiline() != this->textBlock->GetMultiline())
	{
    	node->Set("multiline", this->textBlock->GetMultiline());
	}
    //multilineBySymbol
	if (baseControl->textBlock->GetMultilineBySymbol() != this->textBlock->GetMultilineBySymbol())
	{
    	node->Set("multilineBySymbol", this->textBlock->GetMultilineBySymbol());
	}
    //fitting - STRING OF INT???
	if (baseControl->textBlock->GetFittingOption() != this->textBlock->GetFittingOption())
	{
    	node->Set("fitting", this->textBlock->GetFittingOption());
	}
    
	// Align
	node->SetNodeToMap("align", loader->GetAlignNodeValue(this->GetAlign()));

	// Draw type. Must be overriden for UITextControls.
	node->Set("drawType", loader->GetDrawTypeNodeValue(this->GetBackground()->GetDrawType()));

    SafeDelete(nodeValue);
	SafeRelease(baseControl);
    
    return node;
}

};