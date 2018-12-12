﻿#pragma once
#include "Object.h"

template <typename t>
class ComponentFactory;
class BaseComponentFactory;
class Component;

class ObjectInitializer
{
public:
	explicit ObjectInitializer(Object& aObject)
	{
		CHECK(IsInGameThread());
		myObject = &aObject;
	}

	ObjectInitializer(const ObjectInitializer&) = delete;
	ObjectInitializer& operator=(const ObjectInitializer&) = delete;

	ObjectInitializer(ObjectInitializer&& aOther)
	{
		// Moved after initialization, this is probably not intended as the receiver would have no use for it.
		ENSURE(!wasInitialized);
		*this = Move(aOther);
	}

	ObjectInitializer& operator=(ObjectInitializer&& aOther)
	{
		CHECK(!aOther.wasMoved);
		wasMoved = false;
		wasInitialized = aOther.wasInitialized;
		myObject = aOther.myObject;
		aOther.wasMoved = true;
		return *this;
	}

	~ObjectInitializer()
	{
		if (!wasMoved)
		{
			InitializeObject();
		}
	}

	template <typename TComponentType>
	TComponentType& AddComponent()
	{
		return CastChecked<TComponentType>(myObject->CreateComponentByID(TypeID<Component>::Resolve<TComponentType>(), &CreateComponentFactory<TComponentType>));
	}

	Object& Initialize()
	{
		Object& obj = *myObject;
		InitializeObject();
		wasMoved = true;
		return obj;
	}

private:
	template <typename TComponentType>
	static BaseComponentFactory* CreateComponentFactory()
	{
		return new ComponentFactory<TComponentType>(TypeID<Component>::Resolve<TComponentType>());
	}

	void InitializeObject();

	Object* myObject;
	bool wasMoved = false;
	bool wasInitialized = false;
};
