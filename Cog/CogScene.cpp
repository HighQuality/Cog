#include "pch.h"
#include "CogScene.h"
#include "Component.h"
#include "BaseComponentFactory.h"
#include "BaseComponentFactoryChunk.h"
#include "Object.h"
#include "ObjectFactory.h"
#include "ObjectInitializer.h"

CogScene::CogScene()
{
	myObjectFactory = new ObjectFactory();
}

CogScene::~CogScene()
{
	for (BaseComponentFactory* factory : myComponentFactories)
		delete factory;
	myComponentFactories.Clear();

	delete myObjectFactory;
	myObjectFactory = nullptr;
}

ObjectInitializer CogScene::CreateObject()
{
	CHECK(IsInGameThread());

	Object& object = myObjectFactory->Allocate();
	object.myScene = GetSubPointer();
	return ObjectInitializer(object);
}

void CogScene::RemoveObject(const Object& object)
{
	myObjectFactory->Return(object);
}

BaseComponentFactory& CogScene::FindOrCreateComponentFactory(const TypeID<Component> aComponentType, BaseComponentFactory*(*aFactoryCreator)())
{
	myComponentFactories.Resize(TypeID<Component>::MaxUnderlyingInteger());

	BaseComponentFactory*& factory = myComponentFactories[aComponentType.GetUnderlyingInteger()];
	if (factory == nullptr)
		factory = aFactoryCreator();
	return *factory;
}

void CogScene::DispatchTick(Time aDeltaTime)
{
	for (BaseComponentFactory* factory : myComponentFactories)
	{
		factory->IterateChunks([aDeltaTime](BaseComponentFactoryChunk& aChunk)
		{
			aChunk.DispatchTick(aDeltaTime);
		});
	}
}

void CogScene::DispatchDraw(RenderTarget& aRenderTarget)
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
