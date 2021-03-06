#ifndef __CREATE_PROPERTY_CONTROL_H__
#define __CREATE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "ComboBox.h"

using namespace DAVA;

class CreatePropertyControlDelegate
{
public:
    virtual void NodeCreated(bool success, const String &name, int32 type, VariantType *defaultValue = NULL) = 0;
};

class CreatePropertyControl: public UIControl, public UITextFieldDelegate, public ComboBoxDelegate
{
    
public:
    
    enum ePropertyType
    {
        EPT_STRING = 0,
        EPT_INT,
        EPT_FLOAT,
        EPT_BOOL,
        
        EPT_COUNT
    };
    
public:
    CreatePropertyControl(const Rect & rect, CreatePropertyControlDelegate *newDelegate);
    virtual ~CreatePropertyControl();
    
    virtual void WillAppear();

    virtual void TextFieldShouldReturn(UITextField * textField);
    virtual void TextFieldShouldCancel(UITextField * textField);
    virtual void TextFieldLostFocus(UITextField * textField);
    virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString);
        
	virtual void OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex);

//     const String GetPropName() const;
//     int32 GetPropType() const;
            
protected:
	int32 GetValueTypeFromTypeIndex(int32 typeIndex);
	int32 GetTypeIndexFromValueType(int32 type);

	void UpdateEditableControls();

	UIStaticText *presetText;
	ComboBox *presetCombo;
	String emptyPresetName;
	String selectedPresetName;

    ComboBox *typeCombo;
    UITextField *nameField;

	bool isPresetMode;
        
    CreatePropertyControlDelegate *delegate;
        
    void OnCancel(BaseObject * object, void * userData, void * callerData);
    void OnCreate(BaseObject * object, void * userData, void * callerData);
};



#endif // __CREATE_PROPERTY_CONTROL_H__