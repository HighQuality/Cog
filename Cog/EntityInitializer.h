#pragma once
#include "Entity.h"

template <typename t>
class ComponentFactory;
class BaseComponentFactory;
class Component;

class EntityInitializer
{
public:
	explicit EntityInitializer(Entity& aEntity)
	{
		CHECK(IsInGameThread());
		myEntity = &aEntity;
	}

	EntityInitializer(const EntityInitializer&) = delete;
	EntityInitializer& operator=(const EntityInitializer&) = delete;

	EntityInitializer(EntityInitializer&& aOther)
	{
		// Moved after initialization, this is probably not intended as the receiver would have no use for it.
		ENSURE(!wasInitialized);
		*this = Move(aOther);
	}

	EntityInitializer& operator=(EntityInitializer&& aOther)
	{
		CHECK(!aOther.wasMoved);
		wasMoved = false;
		wasInitialized = aOther.wasInitialized;
		myEntity = aOther.myEntity;
		aOther.wasMoved = true;
		return *this;
	}

	~EntityInitializer()
	{
		if (!wasMoved)
		{
			InitializeEntity();
		}
	}

	template <typename TComponentType>
	TComponentType& AddComponent()
	{
		return reinterpret_cast<TComponentType&>(myEntity->CreateComponentByID(TypeID<Component>::Resolve<TComponentType>()));
	}

	template <typename TComponentType>
	TComponentType& FindOrAddComponent()
	{
		if (TComponentType* component = myEntity->TryGetComponent<TComponentType>())
			return *component;
		return AddComponent<TComponentType>();
	}

	Entity& Initialize()
	{
		Entity& entity = *myEntity;
		InitializeEntity();
		wasMoved = true;
		return entity;
	}

private:
	void InitializeEntity();

	Entity* myEntity;
	bool wasMoved = false;
	bool wasInitialized = false;
};
