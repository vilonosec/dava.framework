#include "Scene3D/Systems/DeleteSystem.h"
#include "Scene3D/SceneNode.h"
#include "Render/RenderManager.h"

namespace DAVA
{
	
DeleteSystem::DeleteSystem()
{
	deletedNodesToAdd = &deletedNodes1;
	deletedNodesToDelete = &deletedNodes2;
}

void DeleteSystem::Process()
{
	uint32 size = deletedNodesToDelete->size();
	for(uint32 i = 0; i < size; ++i)
	{
		DVASSERT(deletedNodesToDelete->at(i)->GetRetainCount() == 1);
		SafeDelete(deletedNodesToDelete->at(i));
	}
	deletedNodesToDelete->clear();

	Vector<SceneNode*> * temp = deletedNodesToAdd;
	deletedNodesToAdd = deletedNodesToDelete;
	deletedNodesToDelete = temp;
}

void DeleteSystem::MarkNodeAsDeleted(SceneNode * node)
{
	deletedNodesToAdd->push_back(node);
}

DeleteSystem::~DeleteSystem()
{
	Process();
}

}