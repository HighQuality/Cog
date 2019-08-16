#pragma once
#include <TypeFundamentals/TypeID.h>
#include "Object.h"

class Component;
class Widget;
class EntityInitializer;
class CogGame;
class BaseComponentFactory;

class Entity final : public Object
{
public:
	using Base = Object;

	Entity();
	~Entity();

	// TODO: Should these return const references if we are const?
	FORCEINLINE Entity* GetParent() const { return myParent; }

	EntityInitializer CreateChild();

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

	virtual bool Destroy() final;

protected:
	friend EntityInitializer;

	void ResolveDependencies(EntityInitializer& aInitializer);
	void Initialize();

private:
	template <typename T>
	friend class Ptr;

	void RemoveWidget(Widget& aWidget);

	CogGame& GetCogGame() const;

	// NOTE: Should only be used from EntityInitializer::AddComponent<TComponentType>
	Component& CreateComponentByID(TypeID<Component> aRequestedTypeID);

	Entity* myParent = nullptr;

	struct ComponentContainer
	{
		Component* component = nullptr;
		bool isInitialRegistration = false;
	};

	Array<Ptr<Widget>> myWidgets;

	// TODO: Change inner array to store at least 1 pointer on the "stack"
	Array<Array<ComponentContainer>> myComponentTypes;
};
