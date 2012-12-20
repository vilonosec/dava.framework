//
//  ControlNodeMetadata.h
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#ifndef __UIEditor__ControlNodeMetadata__
#define __UIEditor__ControlNodeMetadata__

#include "BaseMetadata.h"
#include "UI/UIControl.h"

#include <QColor>

namespace DAVA {

// Metadata common for all DAVA UI Controls.
class UIControlMetadata : public BaseMetadata
{
    Q_OBJECT
    
    // Properties which are common for all UIControls.
    Q_PROPERTY(QString UIControlClassName READ GetUIControlClassName);
    
    Q_PROPERTY(QString Name READ GetName WRITE SetName);
    Q_PROPERTY(int Tag READ GetTag WRITE SetTag);
    
    // Size and position.
    Q_PROPERTY(float RelativeX READ GetRelativeX WRITE SetRelativeX);
    Q_PROPERTY(float RelativeY READ GetRelativeY WRITE SetRelativeY);

    Q_PROPERTY(float AbsoluteX READ GetAbsoluteX WRITE SetAbsoluteX);
    Q_PROPERTY(float AbsoluteY READ GetAbsoluteY WRITE SetAbsoluteY);

    Q_PROPERTY(float SizeX READ GetSizeX WRITE SetSizeX);
    Q_PROPERTY(float SizeY READ GetSizeY WRITE SetSizeY);

    Q_PROPERTY(float PivotX READ GetPivotX WRITE SetPivotX);
    Q_PROPERTY(float PivotY READ GetPivotY WRITE SetPivotY);

    Q_PROPERTY(float Angle READ GetAngle WRITE SetAngle);

    // Background Properties.
    Q_PROPERTY(QColor BackgroundColor READ GetColor WRITE SetColor);
    
    Q_PROPERTY(QString Sprite READ GetSprite WRITE SetSprite);
    Q_PROPERTY(int SpriteFrame READ GetSpriteFrame WRITE SetSpriteFrame);
    
    Q_PROPERTY(int DrawType READ GetDrawType WRITE SetDrawType);
    Q_PROPERTY(int ColorInheritType READ GetColorInheritType WRITE SetColorInheritType);
    Q_PROPERTY(int Align READ GetAlign WRITE SetAlign);
    
    // Flag Properties
    Q_PROPERTY(bool Selected READ GetSelected WRITE SetSelected);
    Q_PROPERTY(bool Visible READ GetVisible WRITE SetVisible);
    Q_PROPERTY(bool Enabled READ GetEnabled WRITE SetEnabled);
    Q_PROPERTY(bool Input READ GetInput WRITE SetInput);
    Q_PROPERTY(bool ClipContents READ GetClipContents WRITE SetClipContents);
    
public:
    UIControlMetadata(QObject* parent = 0);
    
    // Apply move/resize for all controls.
    virtual void ApplyMove(const Vector2& moveDelta);
    virtual void ApplyResize(const Rect& originalRect, const Rect& newRect);

protected:
    virtual QString GetUIControlClassName() { return "UIControl"; };

    // Getters/setters.
    QString GetName() const;
    void SetName(const QString& name);
    
    int GetTag() const;
    void SetTag(int tag);
    
    float GetRelativeX() const;
    void SetRelativeX(float value);
    float GetRelativeY() const;
    void SetRelativeY(float value);

    float GetAbsoluteX() const;
    void SetAbsoluteX(float value);
    float GetAbsoluteY() const;
    void SetAbsoluteY(float value);

    float GetSizeX() const;
    void SetSizeX(float value);
    float GetSizeY() const;
    void SetSizeY(float value);

    float GetPivotX() const;
    void SetPivotX(float value);
    float GetPivotY() const;
    void SetPivotY(float value);

    float GetAngle() const;
    void SetAngle(float value);

    //Drawing flags getters/setters. Virtual because their implementation is different
    //for different control types.
    virtual int GetDrawType();
    virtual void SetDrawType(int value);
    
    virtual int GetColorInheritType();
    virtual void SetColorInheritType(int value);
    
    virtual int GetAlign();
    virtual void SetAlign(int value);

    //Color getter/setter. Also virtual.
    virtual QColor GetColor();
    virtual void SetColor(const QColor& value);

    // Sprite getter/setter. Also virtual one - its implementation is different
    // for different control types.
    virtual void SetSprite(const QString& value);
    virtual QString GetSprite();

    virtual void SetSpriteFrame(int value);
    virtual int GetSpriteFrame();

    //Boolean gettes/setters
    bool GetSelected() const;
    void SetSelected(const bool value);

    bool GetVisible() const;
    void SetVisible(const bool value);

    bool GetEnabled() const;
    void SetEnabled(const bool value);

    bool GetInput() const;
    void SetInput(const bool value);

    bool GetClipContents() const;
    void SetClipContents(const bool value);
};
    
}

#endif /* defined(__UIEditor__ControlNodeMetadata__) */