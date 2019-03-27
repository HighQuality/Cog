#include "pch.h"
#include "CogGame.h"
#include "Stopwatch.h"
#include "Component.h"
#include "Entity.h"
#include "ComponentList.h"
#include "ResourceManager.h"
#include <Program.h>
#include <Await.h>
#include "Transform2D.h"

CogGame* CogGame::ourGame;

bool IsInGameThread()
{
	return GetGame().IsInGameThread();
}

CogGame::CogGame()
	: myGameThreadID(ThreadID::Get()),
	myEntityFactory(new Factory<Entity>()),
	myFrameData(new FrameData())
{
	// Multiple game instances are not allowed under 1 process
	CHECK(!ourGame);
	ourGame = this;

	// Destroyed objects are scheduled for the entirety of the current and next frame before being destroyed
	myScheduledDestroys.Resize(2);
}

CogGame::~CogGame()
{
	myEntityFactory->ReturnAll();

	for (BaseComponentFactory* factory : myComponentFactories)
		delete factory;
	myComponentFactories.Clear();

	delete myEntityFactory;

	delete myComponentList;
	myComponentList = nullptr;

	delete myFrameData;
	myFrameData = nullptr;

	ourGame = nullptr;
}

void CogGame::Run()
{
	CreateResourceManager();

	Stopwatch watch;
	bool isFirstFrame = true;

	while (ShouldKeepRunning())
	{
		Time deltaTime;

		if (isFirstFrame)
		{
			deltaTime = Time::Seconds(1.f / 60.f);
			isFirstFrame = false;
		}
		else
		{
			deltaTime = watch.GetElapsedTime();
		}

		watch.Restart();

		if (myResourceManager)
			myResourceManager->Tick();

		SynchronizedTick(deltaTime);
		TickDestroys();
	}
}

void CogGame::SynchronizedTick(const Time & aDeltaTime)
{
	QueueDispatchers(aDeltaTime);

	// Execute this frame's work
	gProgram->Run(false);

	FindOrCreateComponentFactory<Transform2D>().IterateChunks([](const FactoryChunk<Transform2D> & aChunk)
		{
			aChunk.GetChunkedData()->SynchronizedTick();
		});
}

void CogGame::QueueDispatchers(const Time & aDeltaTime)
{
	UpdateFrameData(*myFrameData, aDeltaTime);

	gProgram->QueueHighPrioWork<CogGame>([](CogGame * aGame)
		{
			aGame->DispatchTick();
		}, this);
}

BaseComponentFactory& CogGame::FindOrCreateComponentFactory(const TypeID<Component> aComponentType)
{
	myComponentFactories.Resize(TypeID<Component>::MaxUnderlyingInteger());

	BaseComponentFactory*& factory = myComponentFactories[aComponentType.GetUnderlyingInteger()];

	if (factory == nullptr)
	{
		const ComponentData& componentData = myComponentList->GetComponentData(aComponentType);

		if (const ComponentData * specializationData = componentData.GetSpecialization())
			factory = specializationData->AllocateFactory();
		else
			factory = componentData.AllocateFactory();
	}

	return *factory;
}

void CogGame::DispatchTick()
{
	gProgram->QueueWork<CogGame>([](CogGame * aGame)
		{
			NO_AWAITS;

			for (BaseComponentFactory* factory : aGame->myComponentFactories)
			{
				if (!factory)
					continue;

				factory->DispatchTick();
			}
		}, this);
}

BaseFactory& CogGame::FindOrCreateObjectFactory(const TypeID<Object> & aObjectType, const FunctionView<BaseFactory * ()> & aFactoryCreator)
{
	CHECK(IsInGameThread());
	const u16 index = aObjectType.GetUnderlyingInteger();
	myObjectFactories.Resize(TypeID<Object>::MaxUnderlyingInteger());
	auto& factory = myObjectFactories[index];
	if (!factory)
		factory = aFactoryCreator();
	return *factory;
}

EntityInitializer CogGame::CreateEntity()
{
	CHECK(IsInGameThread());
	return EntityInitializer(AllocateEntity());
}

Entity& CogGame::AllocateEntity()
{
	CHECK(IsInGameThread());
	return *static_cast<Entity*>(myEntityFactory->AllocateRawObject());
}

void CogGame::CreateResourceManager()
{
	myResourceManager = CreateObject<ResourceManager>();
}

void CogGame::AssignComponentList(const ComponentList & aComponents)
{
	myComponentList = &aComponents;
}

void CogGame::UpdateFrameData(FrameData & aData, const Time & aDeltaTime)
{
	aData.deltaTime = aDeltaTime;
}

void CogGame::TickDestroys()
{
	Array<Object*> destroyNow;

	{
		std::unique_lock<std::mutex> lck(myDestroyMutex);

		if (myScheduledDestroys.GetLength() == 0)
			return;

		destroyNow = Move(myScheduledDestroys[0]);


		for (i32 i = 1; i < myScheduledDestroys.GetLength(); ++i)
		{
			myScheduledDestroys[i - 1] = Move(myScheduledDestroys[i]);
		}
	}

	for (Object* obj : destroyNow)
		obj->ReturnToAllocator();

	destroyNow.Empty();

	{
		std::unique_lock<std::mutex> lck(myDestroyMutex);
		myScheduledDestroys.Last() = Move(destroyNow);
	}
}

void CogGame::ScheduleDestruction(Object& aObject)
{
	CHECK(!aObject.IsPendingDestroy());
	aObject.myChunk->MarkPendingDestroy(aObject.myChunkIndex);

	std::unique_lock<std::mutex> lck(myDestroyMutex);
	myScheduledDestroys.Last().Add(&aObject);
}
