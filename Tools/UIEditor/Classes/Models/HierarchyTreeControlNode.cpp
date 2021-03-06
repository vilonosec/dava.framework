//
//  HierarchyTreeControlNode.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#include "HierarchyTreeControlNode.h"

HierarchyTreeControlNode::HierarchyTreeControlNode(HierarchyTreeNode* parent,
												   UIControl* uiObject,
												   const QString& name) :
	HierarchyTreeNode(name)
{
	this->parent = parent;
	
	this->uiObject = uiObject;
	this->parentUIObject = NULL;
	this->needReleaseUIObjects = false;

	AddControlToParent();
}

HierarchyTreeControlNode::HierarchyTreeControlNode(HierarchyTreeNode* parent,
												   const HierarchyTreeControlNode* node):
	HierarchyTreeNode(node)
{
	this->parent = parent;
	this->uiObject = node->GetUIObject()->Clone();
	this->needReleaseUIObjects = false;
	
	// Remove real children & subcontrols - each control is responsible for its
	// subcontrols by itself.
	const List<UIControl* > &realChildren = GetUIObject()->GetRealChildrenAndSubcontrols();
	for (List<UIControl* >::const_iterator iter = realChildren.begin();
		 iter != realChildren.end();
		 ++iter)
	{
        GetUIObject()->RemoveControl(*iter);
	}
	
	AddControlToParent();
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& child = node->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = child.begin();
		 iter != child.end();
		 ++iter)
	{
		const HierarchyTreeControlNode* controlNode = dynamic_cast<const HierarchyTreeControlNode*>((*iter));
		if (!controlNode)
			continue;
				
		AddTreeNode(new HierarchyTreeControlNode(this, controlNode));
	}
}

void HierarchyTreeControlNode::AddControlToParent()
{
	if (!parent)
		return;
	
	DVASSERT(uiObject);
	
	HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(parent);
	if (screenNode)
	{
		screenNode->GetScreen()->AddControl(this->uiObject);
	}
	else
	{
		//HierarchyTreeControlNode* controlNode = GetControlNode();
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(parent);
		if (controlNode)
		{
			UIControl* control = controlNode->GetUIObject();
			if (control)
				control->AddControl(this->uiObject);
		}
	}	
}

HierarchyTreeControlNode::~HierarchyTreeControlNode()
{
	Cleanup();
	
	UIControl* parent = uiObject->GetParent();
	if (parent)
		parent->RemoveControl(uiObject);
	else
		SafeRelease(uiObject);

	if (needReleaseUIObjects)
	{
		SafeRelease(uiObject);
		SafeRelease(parentUIObject);
	}
}

HierarchyTreeScreenNode* HierarchyTreeControlNode::GetScreenNode() const
{
	if (!parent)
		return NULL;
	
	HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(parent);
	if (screenNode)
		return screenNode;
	
	HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(parent);
	if (controlNode)
		return controlNode->GetScreenNode();
	
	return NULL;
}

HierarchyTreeControlNode* HierarchyTreeControlNode::GetControlNode() const
{
	HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(parent);
	if (controlNode)
		return controlNode;
	
	return NULL;
}

void HierarchyTreeControlNode::SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter)
{
	if (this == insertAfter)
		return;
	
	if (parent)
		parent->RemoveTreeNode(this, false, false);
	
	HierarchyTreeControlNode* newParentControl = dynamic_cast<HierarchyTreeControlNode* >(node);
	HierarchyTreeScreenNode* newParentScreen = dynamic_cast<HierarchyTreeScreenNode* >(node);
	DVASSERT(newParentControl || newParentScreen);
	if (!newParentControl && !newParentScreen)
		return;

	UIControl* afterControl = NULL;
	HierarchyTreeControlNode* insertAfterControl = dynamic_cast<HierarchyTreeControlNode* >(insertAfter);
	if (insertAfterControl)
		afterControl = insertAfterControl->GetUIObject();
	
	UIControl* newParentUI = NULL;
	if (newParentControl)
		newParentUI = newParentControl->GetUIObject();
	else if (newParentScreen)
		newParentUI = newParentScreen->GetScreen();
	
	node->AddTreeNode(this, insertAfter);
	if (newParentUI && uiObject)
	{
		if (insertAfter != node)
		{
			newParentUI->InsertChildAbove(uiObject, afterControl);
		}
		else
		{
			UIControl* belowControl = NULL;
			const List<UIControl*> & controls = newParentUI->GetChildren();
			if (controls.size())
			{
				belowControl = *controls.begin();
			}
			newParentUI->InsertChildBelow(uiObject, belowControl);
		}
	}
	
	parent = node;
}

Vector2 HierarchyTreeControlNode::GetParentDelta(bool skipControl/* = false*/) const
{
	Vector2 parentDelta(0, 0);
	
	if (!skipControl && uiObject)
	{
		Rect rect = uiObject->GetRect();
		parentDelta = rect.GetPosition();
	}
	
	const HierarchyTreeControlNode* parentControl = dynamic_cast<const HierarchyTreeControlNode* >(parent);
	if (parentControl)
		parentDelta += parentControl->GetParentDelta();
	return parentDelta;
}

void HierarchyTreeControlNode::RemoveTreeNodeFromScene()
{
	if (!this->GetParent() || !uiObject->GetParent())
	{
		return;
	}
	
	this->parentUIObject = uiObject->GetParent();
	SafeRetain(this->parentUIObject);

	// Determine the "child above" to return node to scene to the correct position.
	this->childUIObjectAbove = NULL;
	for (List<UIControl*>::const_iterator iter = parentUIObject->GetChildren().begin();
		 iter != parentUIObject->GetChildren().end(); iter ++)
	{
		if (((*iter) == uiObject) && (iter != parentUIObject->GetChildren().begin()))
		{
			iter --;
			this->childUIObjectAbove = (*iter);
			break;
		}
	}

	SafeRetain(uiObject);
	this->parentUIObject->RemoveControl(uiObject);
	
	// We added an additional reference to uiObject and parentUIObject - don't forget
	// to release them in destructor.
	this->needReleaseUIObjects = true;
}

void HierarchyTreeControlNode::ReturnTreeNodeToScene()
{
	if (!this->uiObject || !this->parentUIObject || !this->redoParentNode)
	{
		return;
	}

	// Need to recover the node previously deleted, taking position into account.
	this->redoParentNode->AddTreeNode(this, redoPreviousNode);

	// Return the object back to the proper position.
	if (childUIObjectAbove == NULL)
	{
		// Set it to the top.
		int childCount = parentUIObject->GetChildren().size();
		if (childCount == 0)
		{
			parentUIObject->AddControl(uiObject);
		}
		else
		{
			parentUIObject->InsertChildAbove(uiObject, parentUIObject->GetChildren().front());
		}
	}
	else
	{
		parentUIObject->InsertChildBelow(uiObject, childUIObjectAbove);
	}

	uiObject->Release();
	parentUIObject->Release();
	
	// We just reset extra references to uiObject and parentUIObject - no additional release needed.
	this->needReleaseUIObjects = false;
}

Rect HierarchyTreeControlNode::GetRect() const
{
	Rect rect;
	if (uiObject)
		rect = uiObject->GetRect(true);

	const HIERARCHYTREENODESLIST& childs = GetChildNodes();
	for (HIERARCHYTREENODESLIST::const_iterator iter = childs.begin(); iter != childs.end(); ++iter)
	{
		const HierarchyTreeControlNode* control = dynamic_cast<const HierarchyTreeControlNode*>(*iter);
		if (!control)
			continue;
		
		Rect controlRect = control->GetRect();
		rect = rect.Combine(controlRect);
	}

	return rect;
}
