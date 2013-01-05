//
//  ChangePropertyCommand.h
//  UIEditor
//
//  Created by Yuri Coder on 10/22/12.
//
//

#ifndef __UIEditor__ChangePropertyCommand__
#define __UIEditor__ChangePropertyCommand__

#include <QString>

#include "UI/UIControl.h"

#include "CommandsController.h"

#include "BaseCommand.h"
#include "MetadataFactory.h"
#include "HierarchyTreeNode.h"
#include "PropertyGridWidgetData.h"
#include "PropertiesHelper.h"
#include "UIControlStateHelper.h"

using namespace DAVA;

// Templated Command Data to store the list of "Tree Node ID - Tree Node Values"
// for different value types.
template<typename T> class ChangePropertyCommandData
{
public:
    ChangePropertyCommandData(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID,
                              const T& treeNodePropertyValue)
    {
        this->treeNodeID = treeNodeID;
        this->treeNodePropertyValue = treeNodePropertyValue;
    }

    HierarchyTreeNode::HIERARCHYTREENODEID GetTreeNodeID() const
    {
        return this->treeNodeID;
    }
    
    const T& GetTreeNodePropertyValue() const
    {
        return treeNodePropertyValue;
    }

private:
    HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID;
    T treeNodePropertyValue;
};

// Template ChangePropertyCommand - common for all types.
template <typename Type>
class ChangePropertyCommand : public BaseCommand
{
public:
    ChangePropertyCommand(BaseMetadata* baseMetadata,
                          const PropertyGridWidgetData& propertyGridWidgetData,
                          Type value, bool allStates = false);
    virtual void Execute();

protected:
    // Build the Command Data based on the type.
    Vector<ChangePropertyCommandData<Type> > BuildCommandData(BaseMetadata* baseMetadata);

    // Get the Metadata with Params attached for the Tree Node passed.
    BaseMetadata* GetMetadataForTreeNode(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID);

    // Get the property name.
    QString GetPropertyName();
    
    // Property Grid Widget data.
    PropertyGridWidgetData propertyGridWidgetData;
    
    // Current value.
    Type curValue;
    
    bool setTextForAllStates;
    
    // Current state.
    UIControl::eControlState curState;
    
    // Command data.
    typedef Vector<ChangePropertyCommandData<Type> > COMMANDDATAVECT;
    typedef typename COMMANDDATAVECT::iterator COMMANDDATAVECTITER;
    
    COMMANDDATAVECT commandData;
};

template<typename Type>
    ChangePropertyCommand<Type>::ChangePropertyCommand(BaseMetadata* baseMetadata,
                                                       const PropertyGridWidgetData& propertyGridWidgetData,
                                                       Type value, bool allStates) :
    propertyGridWidgetData(propertyGridWidgetData)
{
    this->curValue = value;
    this->setTextForAllStates = allStates;
    this->curState = baseMetadata->GetUIControlState();
    this->commandData = BuildCommandData(baseMetadata);
}

template<typename Type>
    inline Vector<ChangePropertyCommandData<Type> > ChangePropertyCommand<Type>::BuildCommandData(BaseMetadata* baseMetadata)
{
    // Take the params and use them for building Command Data.
    const METADATAPARAMSVECT& paramsVect = baseMetadata->GetParams();
    Vector< ChangePropertyCommandData<Type> > commandData;

    int paramsCount = baseMetadata->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        const BaseMetadataParams& params = paramsVect[i] ;
        HierarchyTreeNode::HIERARCHYTREENODEID nodeID = params.GetTreeNodeID();
        Type nodeValue = PropertiesHelper::GetPropertyValue<Type>(baseMetadata, GetPropertyName(), i);
        
        commandData.push_back(ChangePropertyCommandData<Type>(nodeID, nodeValue));
    }
    
    return commandData;
}

template<typename Type>
    BaseMetadata* ChangePropertyCommand<Type>::GetMetadataForTreeNode(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID)
{
    const HierarchyTreeNode* treeNode = HierarchyTreeController::Instance()->GetTree().GetNode(treeNodeID);
    if (treeNode == NULL)
    {
        Logger::Error("Tree Node is NULL for Tree Node ID %i", treeNodeID);
        return NULL;
    }
    
    UIControl* uiControl = NULL;
    const HierarchyTreeControlNode* controlNode = dynamic_cast<const HierarchyTreeControlNode*>(treeNode);
    if (controlNode)
    {
        uiControl = controlNode->GetUIObject();
    }
    
    BaseMetadata* baseMetadata = MetadataFactory::Instance()->GetMetadataForTreeNode(treeNode);
    if (baseMetadata == NULL)
    {
        Logger::Error("Unable to found Hierarchy Tree Node Metadata for node %i while executing Command!",
                      treeNodeID);
        return NULL;
    }
    
    METADATAPARAMSVECT params;
    params.push_back(BaseMetadataParams(treeNodeID, uiControl));
    baseMetadata->SetupParams(params);
    
    // Restore the Metadata UI Control state.
    baseMetadata->SetUIControlState(curState);
    
    return baseMetadata;
}

template<typename Type>
    void ChangePropertyCommand<Type>::Execute()
{
    for (COMMANDDATAVECTITER iter = this->commandData.begin(); iter != commandData.end(); iter ++)
    {
        QString propertyName = GetPropertyName();
        BaseMetadata* baseMetadata = GetMetadataForTreeNode((*iter).GetTreeNodeID());
        
        if (setTextForAllStates)
        {
            //Set property text for all states
            int statesCount = UIControlStateHelper::GetUIControlStatesCount();
            for (int stateID = 0; stateID < statesCount; stateID ++)
            {
                UIControl::eControlState state = UIControlStateHelper::GetUIControlState(stateID);
                //Change control state and set text property value
                baseMetadata->SetUIControlState(state);
                PropertiesHelper::SetAllPropertyValues<Type>(baseMetadata, propertyName, curValue);
            }
        }
        else
        {
            PropertiesHelper::SetAllPropertyValues<Type>(baseMetadata, propertyName, curValue);
        }
                
        // Verify whether the properties were indeed changed.
        bool isPropertyValueDiffers = false;
        Type realValue = PropertiesHelper::GetAllPropertyValues<Type>(baseMetadata, propertyName,
                                                                   isPropertyValueDiffers);
        
        bool propertySetOK = (realValue == curValue);
        SAFE_DELETE(baseMetadata);
        
        if (propertySetOK)
        {
            CommandsController::Instance()->EmitChangePropertySucceeded(propertyName);
        }
        else
        {
            CommandsController::Instance()->EmitChangePropertyFailed(propertyName);
        }
    }
}

template<typename Type>
    QString ChangePropertyCommand<Type>::GetPropertyName()
{
    return this->propertyGridWidgetData.getProperty().name();
}

#endif /* defined(__UIEditor__ChangePropertyCommand__) */