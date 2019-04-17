#include "pch.h"
#include "Entity.h"
#include "Component.h"
#include "Widget.h"
#include "ComponentFactory.h"

Entity::Entity()
{
}

Entity::~Entity()
{
	for (auto& componentList : myComponentTypes)
	{
		for (const ComponentContainer& container : componentList)
		{
			if (!container.isInitialRegistration)
				continue;
			container.component->myChunk->ReturnByIndex(container.component->myChunkIndex);
		}
		componentList.Clear();
	}
	myComponentTypes.Clear();
}

static thread_local Array<Component*> newComponents;
static thread_local bool isResolvingDependencies = false;

bool Entity::Destroy()
{
	if (!Base::Destroy())
		return false;

	for (const Ptr<Widget>& widget : myWidgets)
	{
		if (widget)
			widget->Destroy();
	}

	return true;
}

void Entity::ResolveDependencies(EntityInitializer& aInitializer)
{
	// Recursion is not supported here
	CHECK(!isResolvingDependencies);

	isResolvingDependencies = true;

	for (const Array<ComponentContainer>& components : myComponentTypes)
	{
		newComponents.PrepareAdd(components.GetLength());

		for (const ComponentContainer& container : components)
		{
			if (!container.isInitialRegistration)
				continue;
			newComponents.Add(container.component);
		}
	}

	i32 oldLength = 0;

	do
	{
		const i32 end = newComponents.GetLength();

		for (i32 i = oldLength; i < end; ++i)
			newComponents[i]->ResolveDependencies(aInitializer);

		oldLength = end;
	}
	while (newComponents.GetLength() > oldLength);

	newComponents.Clear();
	isResolvingDependencies = false;
}

void Entity::Initialize()
{
	ForEachComponent<Component>([](Component& aComponent)
	{
		aComponent.Initialize();
	});
}

void Entity::RemoveWidget(Widget& aWidget)
{
	CHECK(myWidgets.RemoveSwap(aWidget));
}

CogGame& Entity::GetCogGame() const
{
	return GetGame();
}

Component& Entity::CreateComponentByID(TypeID<Component> aRequestedTypeID)
{
	BaseComponentFactory& factory = GetGame().FindOrCreateComponentFactory(aRequestedTypeID);
	Component& component = factory.AllocateGeneric();
	component.SetEntity(this);

	const TypeID<Component> createdTypeID = factory.GetTypeID();

	myComponentTypes.Resize(TypeID<Component>::MaxUnderlyingInteger());

	ComponentContainer container;
	container.component = &component;
	container.isInitialRegistration = true;
	
	myComponentTypes[createdTypeID.GetUnderlyingInteger()].Add(container);

	if (isResolvingDependencies)
	{
		newComponents.Add(&component);
	}

	component.GetBaseClasses([this, &component](const TypeID<Component>& aType)
	{
		const u16 typeIndex = aType.GetUnderlyingInteger();
		if (typeIndex >= myComponentTypes.GetLength())
			myComponentTypes.Resize(typeIndex + 1);

		ComponentContainer container;
		container.component = &component;
		container.isInitialRegistration = false;
		myComponentTypes[typeIndex].Add(Move(container));
	});

	return component;
}

EntityInitializer Entity::CreateChild()
{
	Entity& entity = GetGame().AllocateEntity();
	entity.myParent = this;
	return EntityInitializer(entity);
}
