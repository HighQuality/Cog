#pragma once
#include "TypeID.h"

class GameWorld;
class Component;
class ObjectFactoryChunk;

class Object
{
public:
	Object();
	~Object();

	FORCEINLINE GameWorld& GetWorld() { return *myWorld; }
	FORCEINLINE const GameWorld& GetWorld() const { return *myWorld; }

	FORCEINLINE bool IsInitialized() const { return myIsInitialized; }

	void CallbackTest(String obj) const
	{
		Println(L"Callback received: ", obj);
	}

	template <typename T>
	const T& Get() const
	{
		return const_cast<Object*>(this)->Get<T>();
	}

	template <typename T>
	T& Get()
	{
		T* component = TryGet<T>();
		CHECK(component);
		return *component;
	}

	template <typename T>
	const T* TryGet() const
	{
		return const_cast<Object*>(this)->TryGet<T>();
	}

	template <typename T>
	T* TryGet()
	{
		const u16 index = TypeID<Component>::Resolve<T>().GetUnderlyingInteger();

		if (!myComponents.IsValidIndex(index) || myComponents[index].GetLength() == 0)
			return nullptr;

		return myComponents[index][0];
	}

private:
	friend GameWorld;
	friend ObjectFactoryChunk;

	template <typename T>
	friend class Ptr;

	GameWorld* myWorld;
	ObjectFactoryChunk* myChunk;
	Array<Array<Component*>> myComponents;

	void RegisterComponent(Component& aComponent, TypeID<Component> typeID)
	{
		myComponents.Resize(TypeID<Component>::MaxUnderlyingInteger());
		myComponents[typeID.GetUnderlyingInteger()].Add(&aComponent);
	}

	bool myIsInitialized = false;
};
