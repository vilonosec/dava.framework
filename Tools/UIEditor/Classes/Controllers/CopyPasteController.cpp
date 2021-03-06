//
//  CopyPasteController.cpp
//  UIEditor
//
//  Created by adebt on 11/5/12.
//
//

#include "CopyPasteController.h"
#include "PasteCommand.h"
#include "CommandsController.h"
#include "HierarchyTreeAggregatorControlNode.h"

using namespace DAVA;

CopyPasteController::CopyPasteController()
{
	Clear();
}

CopyPasteController::~CopyPasteController()
{
	Clear();
}

CopyPasteController::CopyType CopyPasteController::GetCopyType() const
{
	return copyType;
}

void CopyPasteController::Copy(const HierarchyTreeNode::HIERARCHYTREENODESLIST& items)
{
	if (!items.size())
		return;
	
	Clear();
	CopyType curCopy = CopyTypeNone;
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		const HierarchyTreeNode* node = (*iter);
		const HierarchyTreeScreenNode* screen = dynamic_cast<const HierarchyTreeScreenNode* >(node);
		const HierarchyTreePlatformNode* platform = dynamic_cast<const HierarchyTreePlatformNode*>(node);
		
		if (curCopy == CopyTypeNone)
		{
			if (platform)
				curCopy = CopyTypePlatform;
			else if (screen)
				curCopy = CopyTypeScreen;
		}
		
		HierarchyTreeNode* copy = NULL;
		if (curCopy == CopyTypePlatform && platform)
		{
			copy = new HierarchyTreePlatformNode(NULL, platform);
		}
		else if (curCopy == CopyTypeScreen && screen)
		{
			copy = new HierarchyTreeScreenNode(NULL, screen);
		}
		
		if (copy)
		{
			this->items.push_back(copy);
		}
	}
	
	if (this->items.size())
		copyType = curCopy;
}

void CopyPasteController::CopyControls(const HierarchyTreeController::SELECTEDCONTROLNODES& items)
{
	if (!items.size())
		return;
	
	Clear();
	for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		//HierarchyTreeNode* item = (*iter);
		const HierarchyTreeControlNode* control = (*iter);
		if (!control)
			continue;

		if (ControlIsChild(items, control))
			continue;
		
		HierarchyTreeControlNode* copy = NULL;
		const HierarchyTreeAggregatorControlNode* aggregatorControl = dynamic_cast<const HierarchyTreeAggregatorControlNode*>(control);
		if (aggregatorControl)
			copy = new HierarchyTreeAggregatorControlNode(NULL, aggregatorControl);
		else
			copy = new HierarchyTreeControlNode(NULL, control);
		
		this->items.push_back(copy);
	}
	if (this->items.size())
		copyType = CopyTypeControl;
}

bool CopyPasteController::ControlIsChild(const HierarchyTreeController::SELECTEDCONTROLNODES& items, const HierarchyTreeControlNode* control) const
{
	bool isChild = false;
	//skip child control when copy parent
	for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		const HierarchyTreeControlNode* parent = (*iter);
		if (parent->IsHasChild(control))
		{
			isChild = true;
			break;
		}
	}
	return isChild;
}

void CopyPasteController::Clear()
{
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode *node = (*iter);
		SAFE_DELETE(node);
	}

	items.clear();
	copyType = CopyTypeNone;
}


void CopyPasteController::Paste(HierarchyTreeNode* parentNode)
{
	if (!parentNode)
		return;
	
	PasteCommand* cmd = new PasteCommand(parentNode, copyType, &items);
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}