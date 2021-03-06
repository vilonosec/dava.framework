#ifndef __DAVAENGINE_UPDATABLE_COMPONENT_H__
#define __DAVAENGINE_UPDATABLE_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"

namespace DAVA
{

class BaseObject;

class IUpdatable
{
public:
	virtual ~IUpdatable() {};
};

class IUpdatableBeforeTransform : public virtual IUpdatable
{
public:
	virtual void UpdateBeforeTransform(float32 timeElapsed) = 0;
};

class IUpdatableAfterTransform : public virtual IUpdatable
{
public:
	virtual void UpdateAfterTransform(float32 timeElapsed) = 0;
};

class UpdatableComponent : public Component
{
public:
	IMPLEMENT_COMPONENT_TYPE(UPDATABLE_COMPONENT);

	enum eUpdateType
	{
		UPDATE_PRE_TRANSFORM,
		UPDATE_POST_TRANSFORM,

		UPDATES_COUNT
	};

	UpdatableComponent();
	virtual Component * Clone(Entity * toEntity);

	void SetUpdatableObject(IUpdatable * updatableObject);
	IUpdatable * GetUpdatableObject();

private:
	IUpdatable * updatableObject;
    
public:
    INTROSPECTION_EXTEND(UpdatableComponent, Component,
        MEMBER(updatableObject, "Updatable Object", INTROSPECTION_SERIALIZABLE)
    );

};

}

#endif //__DAVAENGINE_UPDATABLE_COMPONENT_H__