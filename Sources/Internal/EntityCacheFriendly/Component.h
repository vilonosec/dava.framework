#ifndef __DAVAENGINE_ENTITY_COMPONENT_H__
#define __DAVAENGINE_ENTITY_COMPONENT_H__

#include "Entity/Pool.h"
#include "Entity/ComponentTypes.h"
#include "Entity/EntityManager.h"
#include <typeinfo>


#define DECLARE_COMPONENT(ComponentName) \
class ComponentName : public Component \
{ \
public:	\
	static ComponentName * Get() { return instance; } \
	\
	static void Create() \
	{   \
		ComponentName * component = new ComponentName(); \
		RegisterComponent(#ComponentName, component); \
		component->Register(); \
	}\
private: \
	static ComponentName * instance; \
	ComponentName() \
	{ \
		instance = this; \
	} \
	inline void Register();	\
\
}; 


#define IMPLEMENT_COMPONENT(ComponentName) \
ComponentName * ComponentName::instance = 0;

namespace DAVA 
{

class Component
{
public:
	static void RegisterComponent(const char * componentName, Component * component); //create or get from cache
	static Component * GetComponent(const char * componentName);
    
    Component()
    {
        //type = ComponentType(); //this duplicates field declaration
        componentsByIndex[type.GetIndex()] = this;
    };
    
    
    template <class T>
    void RegisterData(const char * name)
    {
        EntityManager::CreatePoolAllocator<T>(name);
        dataNames.insert(name);
    }

//	template<class T>
//	TemplatePool<T>* CreatePool(T a, const char * name)
//	{
//		TemplatePool<T> * pool = new TemplatePool<T>(100);
//		pools.push_back(pool);
//        return pool;
//	}
//    

    const ComponentType & GetType() { return type; };
    
    static Component * GetComponentByIndex(uint64 index);
    
    Set<const char*> & GetDataNames() {return dataNames; };

	static Map<const char *, Component * > cache;//<name, component>
    
private:
	ComponentType type;
    Set<const char*> dataNames;
    static Map<uint64, Component*>  componentsByIndex;
};
    
};


#endif // __DAVAENGINE_ENTITY_COMPONENT_H__
