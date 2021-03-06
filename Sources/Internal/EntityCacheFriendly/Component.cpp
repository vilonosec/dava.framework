#include "Entity/Component.h"
#include "Entity/EntityManager.h"

namespace DAVA 
{

Map<const char *, Component * > Component::cache;
Map<uint64, Component*>  Component::componentsByIndex;

Component * Component::GetComponentByIndex(uint64 index)
{
    Map<uint64, Component*>::iterator it = componentsByIndex.find(index);
    if (it != componentsByIndex.end())
    {
        return it->second;
    }
    return 0;
}
    
void Component::RegisterComponent(const char * componentName, Component * component)
{
    cache[componentName] = component;
}
    
Component * Component::GetComponent(const char * componentName)
{
    Map<const char *, Component * >::iterator res = cache.find(componentName);
    if (res != cache.end())
    {
        return res->second;
    }
    return 0;
}

    
};

