#include "pch.h"
#include "GameWorld.h"
#include "Component.h"
#include "BaseComponentFactory.h"
#include "BaseComponentFactoryChunk.h"
#include "Object.h"
#include "ObjectFactory.h"

GameWorld::GameWorld()
{
	myObjectFactory = new ObjectFactory();
}

GameWorld::~GameWorld()
{
	for (BaseComponentFactory* factory : myComponentFactories)
		delete factory;
	myComponentFactories.Clear();

	delete myObjectFactory;
	myObjectFactory = nullptr;
}

Object& GameWorld::CreateObject()
{
	Object& object = myObjectFactory->Allocate();
	object.myWorld = this;
	return object;
}

Component& GameWorld::CreateComponentOnObjectFromFactory(BaseComponentFactory& aComponentFactory, Object& aObject)
{
	// Can't add components to already initialized objects
	CHECK(!aObject.IsInitialized());

	Component& component = aComponentFactory.AllocateGeneric();
	component.myObject = &aObject;
	aObject.RegisterComponent(component, aComponentFactory.GetTypeID());
	return component;
}

void GameWorld::DispatchTick(Time aDeltaTime)
{
	for (BaseComponentFactory* factory : myComponentFactories)
	{
		factory->IterateChunks([aDeltaTime](BaseComponentFactoryChunk& aChunk)
		{
			aChunk.DispatchTick(aDeltaTime);
		});
	}
}

void GameWorld::DispatchDraw(RenderTarget& aRenderTarget)
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
