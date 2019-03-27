#include "pch.h"
#include "CogGame.h"
#include "Stopwatch.h"
#include "Component.h"
#include "BaseComponentFactory.h"
#include "BaseComponentFactoryChunk.h"
#include "Entity.h"
#include "EntityFactory.h"
#include <Semaphore.h>
#include "ComponentList.h"
#include "ResourceManager.h"
#include <Program.h>
#include <Await.h>

CogGame* CogGame::ourGame;

bool IsInGameThread()
{
	return GetGame().IsInGameThread();
}

CogGame::CogGame()
	: myGameThreadID(ThreadID::Get()),
	myEntityFactory(new EntityFactory()),
	myFrameData(new FrameData())
{
	// Multiple game instances are not allowed under 1 process
	CHECK(!ourGame);
	ourGame = this;
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
	}
}

void CogGame::SynchronizedTick(const Time& aDeltaTime)
{
	QueueDispatchers(aDeltaTime);

	// Execute this frame's work
	gProgram->Run(false);
}

void CogGame::QueueDispatchers(const Time& aDeltaTime)
{
	UpdateFrameData(*myFrameData, aDeltaTime);

	gProgram->QueueHighPrioWork<CogGame>([](CogGame* aGame)
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
		
		if (const ComponentData* specializationData = componentData.GetSpecialization())
			factory = specializationData->AllocateFactory();
		else
			factory = componentData.AllocateFactory();
	}

	return *factory;
}

void CogGame::DispatchTick()
{
	gProgram->QueueHighPrioWork<FrameData>([](FrameData* aTickData)
	{
		NO_AWAITS;

		CogGame& game = GetGame();

		for (BaseComponentFactory* factory : game.myComponentFactories)
		{
			if (!factory)
				continue;
			
			factory->IterateChunks([aTickData](BaseComponentFactoryChunk& aChunk)
			{
				aChunk.DispatchTick(*aTickData);
			});
		}
	}, myFrameData);
}

BaseObjectFactory& CogGame::FindOrCreateObjectFactory(const TypeID<Object>& aObjectType, const FunctionView<BaseObjectFactory*()>& aFactoryCreator)
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
	return myEntityFactory->Allocate();
}

void CogGame::CreateResourceManager()
{
	myResourceManager = CreateObject<ResourceManager>();
}

void CogGame::AssignComponentList(const ComponentList& aComponents)
{
	myComponentList = &aComponents;
}

void CogGame::UpdateFrameData(FrameData& aData, const Time& aDeltaTime)
{
	aData.deltaTime = aDeltaTime;
}
