#pragma once
#include "TypeID.h"

class BaseComponentFactory;
class GameWorld;
class CogGameWorld;
class Component;
class ObjectFactoryChunk;

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
	FORCEINLINE GameWorld& GetWorld() const { return *myWorld; }
	FORCEINLINE CogGameWorld& GetCogWorld() const { return *reinterpret_cast<CogGameWorld*>(myWorld); }

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
	friend class ObjectInitializer;

	void ResolveDependencies();
	void Initialize();

private:
	friend CogGameWorld;
	friend ObjectFactoryChunk;

	template <typename T>
	friend class Ptr;

	template <typename TComponentType>
	static BaseComponentFactory* CreateComponentFactory()
	{
		return new ComponentFactory<TComponentType>(TypeID<Component>::Resolve<TComponentType>());
	}

	// NOTE: Should only be used by ObjectInitializer class
	template <typename TComponentType>
	TComponentType& AddComponent()
	{
		return CastChecked<TComponentType>(AddComponent(TypeID<Component>::Resolve<TComponentType>(), &CreateComponentFactory<TComponentType>));
	}

	// NOTE: Should only be used from Component::AddComponent<TComponentType>
	Component& AddComponent(TypeID<Component> aComponentID, BaseComponentFactory*(*aFactoryCreator)());

	GameWorld* myWorld;
	ObjectFactoryChunk* myChunk;

	// TODO: Change inner array to store at least 1 pointer on the "stack"
	Array<Array<Component*>> myComponentTypes;
};
