#include "pch.h"
#include "CogGameWorld.h"
#include "Component.h"
#include "BaseComponentFactory.h"
#include "BaseComponentFactoryChunk.h"
#include "Object.h"
#include "ObjectFactory.h"
#include "ObjectInitializer.h"

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

ObjectInitializer CogGameWorld::CreateObject()
{
	CHECK(IsInGameThread());

	Object& object = myObjectFactory->Allocate();
	object.myWorld = GetSubPointer();
	return ObjectInitializer(object);
}

void CogGameWorld::RemoveObject(const Object& object)
{
	myObjectFactory->Return(object);
}

BaseComponentFactory& CogGameWorld::FindOrCreateComponentFactory(const TypeID<Component> aComponentType, BaseComponentFactory*(*aFactoryCreator)())
{
	myComponentFactories.Resize(TypeID<Component>::MaxUnderlyingInteger());

	BaseComponentFactory*& factory = myComponentFactories[aComponentType.GetUnderlyingInteger()];
	if (factory == nullptr)
		factory = aFactoryCreator();
	return *factory;
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
