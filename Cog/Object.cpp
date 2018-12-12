#include "pch.h"
#include "Object.h"
#include "Component.h"
#include "CogGameWorld.h"

Object::Object()
{
}

Object::~Object()
{
}

static thread_local Array<Component*> newComponents;
static thread_local Array<i32> lengths;
static thread_local bool isResolvingDependencies = false;

void Object::ResolveDependencies()
{
	// Recursion is not supported here
	CHECK(!isResolvingDependencies);

	isResolvingDependencies = true;

	for (const Array<Component*>& components : myComponentTypes)
		newComponents.Append(components);

	i32 oldLength = 0;

	do
	{
		const i32 start = oldLength;
		const i32 end = newComponents.GetLength();

		for (i32 i = start; i < end; ++i)
			newComponents[i]->ResolveDependencies();

		oldLength = end;
	} while (newComponents.GetLength() > oldLength);

	newComponents.Clear();
	isResolvingDependencies = false;
}

void Object::Initialize()
{
}

Component& Object::AddComponent(TypeID<Component> aComponentID, BaseComponentFactory*(*aFactoryCreator)())
{
	BaseComponentFactory& factory = GetCogWorld().FindOrCreateComponentFactory(aComponentID, aFactoryCreator);
	Component& component = factory.AllocateGeneric();

	myComponentTypes.Resize(TypeID<Component>::MaxUnderlyingInteger());
	myComponentTypes[aComponentID.GetUnderlyingInteger()].Add(&component);

	if (isResolvingDependencies)
	{
		newComponents.Add(&component);
	}

	return component;
}
