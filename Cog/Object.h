#pragma once
#include "TypeID.h"

class BaseComponentFactory;
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
	FORCEINLINE Object& GetParent() const { return *myParent; }
	FORCEINLINE Object* TryGetParent() const { return myParent; }
	FORCEINLINE bool HasParent() const { return myParent != nullptr; }

	ObjectInitializer CreateChild();
	void Destroy();

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

		return myComponentTypes[index][0].component;
	}

	template <typename T, typename TCallback>
	void ForEachComponent(const TCallback& aCallback)
	{
		const u16 componentID = TypeID<Component>::Resolve<T>().GetUnderlyingInteger();

		if (componentID >= myComponentTypes.GetLength())
			return;

		for (const ComponentContainer& componentContainer : myComponentTypes[componentID])
			aCallback(reinterpret_cast<T&>(*componentContainer.component));
	}

	template <typename T, typename TCallback>
	void ForEachComponent(const TCallback& aCallback) const
	{
		const u16 componentID = TypeID<Component>::Resolve<T>().GetUnderlyingInteger();

		if (componentID >= myComponentTypes.GetLength())
			return;

		for (const ComponentContainer& componentContainer : myComponentTypes[componentID])
			aCallback(reinterpret_cast<const T&>(*componentContainer.component));
	}

protected:
	friend ObjectInitializer;

	void ResolveDependencies(ObjectInitializer& aInitializer);
	void Initialize();

private:
	friend ObjectFactoryChunk;

	template <typename T>
	friend class Ptr;

	// NOTE: Should only be used from ObjectInitializer::AddComponent<TComponentType>
	Component& CreateComponentByID(TypeID<Component> aComponentID);

	Object* myParent = nullptr;
	ObjectFactoryChunk* myChunk = nullptr;

	struct ComponentContainer
	{
		Component* component = nullptr;
		bool isInitialRegistration = false;
	};

	// TODO: Change inner array to store at least 1 pointer on the "stack"
	Array<Array<ComponentContainer>> myComponentTypes;
};
