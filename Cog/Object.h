#pragma once
#include "TypeID.h"

class BaseComponentFactory;
class Scene;
class CogScene;
class Component;
class ObjectFactoryChunk;
class ObjectInitializer;

template <typename T>
class ComponentFactory;

class Object final
{
public:
	Object();
	~Object();

	Object(const Object&) = delete;
	Object(Object&&) = delete;

	Object& operator=(const Object&) = delete;
	Object& operator=(Object&&) = delete;

	// TODO: Should these return const references if we are const?
	FORCEINLINE Scene& GetScene() const { return *myScene; }
	FORCEINLINE CogScene& GetCogScene() const { return *reinterpret_cast<CogScene*>(myScene); }

	template <typename T>
	const T& GetComponent() const
	{
		return const_cast<Object*>(this)->GetComponent<T>();
	}

	template <typename T>
	T& GetComponent()
	{
		T* component = TryGetComponent<T>();
		CHECK(component);
		return *component;
	}

	template <typename T>
	const T* TryGetComponent() const
	{
		return const_cast<Object*>(this)->TryGetComponent<T>();
	}

	template <typename T>
	T* TryGetComponent()
	{
		const u16 index = TypeID<Component>::Resolve<T>().GetUnderlyingInteger();

		if (!myComponentTypes.IsValidIndex(index) || myComponentTypes[index].GetLength() == 0)
			return nullptr;

		return myComponentTypes[index][0];
	}

protected:
	friend ObjectInitializer;

	void ResolveDependencies(ObjectInitializer& aInitializer);
	void Initialize();

private:
	friend CogScene;
	friend ObjectFactoryChunk;

	template <typename T>
	friend class Ptr;

	// NOTE: Should only be used from Component::AddComponent<TComponentType>
	Component& CreateComponentByID(TypeID<Component> aComponentID, BaseComponentFactory*(*aFactoryCreator)());

	Scene* myScene;
	ObjectFactoryChunk* myChunk;

	// TODO: Change inner array to store at least 1 pointer on the "stack"
	Array<Array<Component*>> myComponentTypes;
};
