#pragma once
#include "Entity.h"

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

	EntityInitializer(EntityInitializer&& aOther) noexcept
	{
		// Moved after initialization, this is probably not intended as the receiver would have no use for it.
		ENSURE(!myWasInitialized);
		*this = Move(aOther);
	}

	EntityInitializer& operator=(EntityInitializer&& aOther) noexcept
	{
		CHECK(!aOther.myWasMoved);
		myWasMoved = false;
		myWasInitialized = aOther.myWasInitialized;
		myEntity = aOther.myEntity;
		aOther.myWasMoved = true;
		return *this;
	}

	~EntityInitializer()
	{
		if (!myWasMoved)
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
		myWasMoved = true;
		return entity;
	}

private:
	void InitializeEntity();

	Entity* myEntity;
	bool myWasMoved = false;
	bool myWasInitialized = false;
};
