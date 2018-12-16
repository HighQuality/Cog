#pragma once
#include "TypeID.h"

class BaseComponentFactory;
class Component;
class Object;
class EntityFactoryChunk;
class EntityInitializer;
class CogGame;

template <typename T>
class ComponentFactory;

class Entity final
{
public:
	Entity();
	~Entity();

	Entity(const Entity&) = delete;
	Entity(Entity&&) = delete;

	Entity& operator=(const Entity&) = delete;
	Entity& operator=(Entity&&) = delete;

	// TODO: Should these return const references if we are const?
	FORCEINLINE Entity& GetParent() const { return *myParent; }
	FORCEINLINE Entity* TryGetParent() const { return myParent; }
	FORCEINLINE bool HasParent() const { return myParent != nullptr; }

	EntityInitializer CreateChild();
	void Destroy();
	
	template <typename T>
	T& CreateWidget()
	{
		T& widget = GetCogGame().template CreateObject<T>();
		widget.OnDestroyed.Subscribe(this, &Entity::RemoveWidget);
		myWidgets.Add(widget);
		return widget;
	}
	
	template <typename T>
	const T& GetComponent() const
	{
		return const_cast<Entity*>(this)->GetComponent<T>();
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
		return const_cast<Entity*>(this)->TryGetComponent<T>();
	}

	template <typename T>
	T* TryGetComponent()
	{
		const u16 index = TypeID<Component>::Resolve<T>().GetUnderlyingInteger();

		if (!myComponentTypes.IsValidIndex(index) || myComponentTypes[index].GetLength() == 0)
			return nullptr;

		return reinterpret_cast<T*>(myComponentTypes[index][0].component);
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
	friend EntityInitializer;

	void ResolveDependencies(EntityInitializer& aInitializer);
	void Initialize();

private:
	friend EntityFactoryChunk;

	template <typename T>
	friend class Ptr;

	void RemoveWidget(Object& aWidget);

	CogGame& GetCogGame() const;

	// NOTE: Should only be used from EntityInitializer::AddComponent<TComponentType>
	Component& CreateComponentByID(TypeID<Component> aComponentID);

	Entity* myParent = nullptr;
	EntityFactoryChunk* myChunk = nullptr;

	struct ComponentContainer
	{
		Component* component = nullptr;
		bool isInitialRegistration = false;
	};

	Array<Ptr<Object>> myWidgets;

	// TODO: Change inner array to store at least 1 pointer on the "stack"
	Array<Array<ComponentContainer>> myComponentTypes;
};
