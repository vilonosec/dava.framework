#ifndef __LODDISTANCE_CONTROL_H__
#define __LODDISTANCE_CONTROL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class LodDistanceControl;
class LodDistanceControlDelegate
{
public:
    
    virtual void DistanceChanged(LodDistanceControl *forControl, int32 index, float32 value) = 0;
};

class LodDistanceControl : public UIControl, public UITextFieldDelegate
{
public:

	LodDistanceControl(LodDistanceControlDelegate *newDelegate, const Rect &rect, bool rectInAbsoluteCoordinates = false);
    virtual ~LodDistanceControl();

    virtual void WillDisappear();
    
    virtual void Update(float32 timeElapsed);

    void SetDistances(float32 *newDistances, int32 *newTriangles, int32 newCount);
    
    virtual void Input(UIEvent *currentInput);
    
    //UITextFieldDelegate
    virtual void TextFieldShouldReturn(UITextField * textField);
    virtual void TextFieldShouldCancel(UITextField * textField);
    virtual void TextFieldLostFocus(UITextField * textField);
	virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, 
                                     int32 replacementLength, const WideString & replacementString);
    
    
    static const float32 GetControlHeightForLodCount(int32 lodCount);
    
private:

    void ReleaseControls();
    void UpdateDistanceToCamera();
    
    UIControl **zones;
    UIControl **sliders;
    
    int32 count;
    float32 distances[LodComponent::MAX_LOD_LAYERS];
    int32 triangles[LodComponent::MAX_LOD_LAYERS];
    
    float32 maxDistance;

    LodDistanceControlDelegate *delegate;
    
    int32 mainTouch;
    float32 oldPos;
	float32 newPos;
    
    UIControl *activeSlider;
    void UpdateSliderPos();
    
    Vector2 leftEdge;
    Vector2 rightEdge;
    
    UIControl *leftZone;
    UIControl *rightZone;
    
    UIStaticText *distanceText[LodComponent::MAX_LOD_LAYERS];
    UITextField *distanceTextValues[LodComponent::MAX_LOD_LAYERS];

    UIStaticText *trianglesText[LodComponent::MAX_LOD_LAYERS];
    UIStaticText *trianglesTextValues[LodComponent::MAX_LOD_LAYERS];

    UIStaticText *distanceToCameraText;
    UIStaticText *distanceToCameraValue;
    
    int32 activeLodIndex;
};


#endif //#ifndef __LODDISTANCE_CONTROL_H__
