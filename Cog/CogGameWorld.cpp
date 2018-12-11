#include "pch.h"
#include "CogGameWorld.h"
#include "Component.h"
#include "BaseComponentFactory.h"
#include "BaseComponentFactoryChunk.h"
#include "Object.h"
#include "ObjectFactory.h"

CogGameWorld::CogGameWorld()
{
	myObjectFactory = new ObjectFactory();
}

CogGameWorld::~CogGameWorld()
{
	for (BaseComponentFactory* factory : myComponentFactories)
		delete factory;
	myComponentFactories.Clear();

	delete myObjectFactory;
	myObjectFactory = nullptr;
}

Object& CogGameWorld::CreateObject()
{
	Object& object = myObjectFactory->Allocate();
	object.myWorld = this;
	return object;
}

void CogGameWorld::RemoveObject(const Object& object)
{
	myObjectFactory->Return(object);
}

Component& CogGameWorld::CreateComponentOnObjectFromFactory(BaseComponentFactory& aComponentFactory, Object& aObject)
{
	// Can't add components to already initialized objects
	CHECK(!aObject.IsInitialized());

	Component& component = aComponentFactory.AllocateGeneric();
	component.myObject = &aObject;
	aObject.RegisterComponent(component, aComponentFactory.GetTypeID());
	return component;
}

void CogGameWorld::DispatchTick(Time aDeltaTime)
{
	for (BaseComponentFactory* factory : myComponentFactories)
	{
		factory->IterateChunks([aDeltaTime](BaseComponentFactoryChunk& aChunk)
		{
			aChunk.DispatchTick(aDeltaTime);
		});
	}
}

void CogGameWorld::DispatchDraw(RenderTarget& aRenderTarget)
{
	for (BaseComponentFactory* factory : myComponentFactories)
	{
		factory->IterateChunks([&aRenderTarget](BaseComponentFactoryChunk& aChunk)
		{
			aChunk.DispatchDraw3D(aRenderTarget);
		});
	}

	for (BaseComponentFactory* factory : myComponentFactories)
	{
		factory->IterateChunks([&aRenderTarget](BaseComponentFactoryChunk& aChunk)
		{
			aChunk.DispatchDraw2D(aRenderTarget);
		});
	}
}
