#include "pch.h"
#include "Object.h"
#include "Component.h"
#include "BaseComponentFactory.h"

Object::Object()
{
}

Object::~Object()
{
	for (const Ptr<Widget>& widget : myWidgets)
	{
		if (widget)
			widget->Destroy();
	}

	for (auto& componentList : myComponentTypes)
	{
		for (const ComponentContainer& container : componentList)
		{
			if (!container.isInitialRegistration)
				continue;
			container.component->myChunk->ReturnByID(container.component->myChunkIndex);
		}
		componentList.Clear();
	}
	myComponentTypes.Clear();

	myChunk = nullptr;
}

static thread_local Array<Component*> newComponents;
static thread_local bool isResolvingDependencies = false;

void Object::ResolveDependencies(ObjectInitializer& aInitializer)
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

void Object::Initialize()
{
	ForEachComponent<Component>([](Component& aComponent)
	{
		aComponent.Initialize();
	});
}

void Object::RemoveWidget(Widget& aWidget)
{
	CHECK(myWidgets.RemoveSwap(aWidget));
}

CogGame& Object::GetCogGame() const
{
	return GetGame();
}

Component& Object::CreateComponentByID(TypeID<Component> aComponentID)
{
	BaseComponentFactory& factory = GetGame().FindOrCreateComponentFactory(aComponentID);
	Component& component = factory.AllocateGeneric();
	component.myChunk->AssignObject(component.myChunkIndex, *this);

	myComponentTypes.Resize(TypeID<Component>::MaxUnderlyingInteger());

	ComponentContainer container;
	container.component = &component;
	container.isInitialRegistration = true;

	myComponentTypes[aComponentID.GetUnderlyingInteger()].Add(container);

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

ObjectInitializer Object::CreateChild()
{
	Object& obj = GetGame().AllocateObject();
	obj.myParent = this;
	return ObjectInitializer(obj);
}

void Object::Destroy()
{
	CHECK(IsInGameThread());

	myChunk->Return(*this);
}
