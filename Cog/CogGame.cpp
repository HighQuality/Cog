#include "pch.h"
#include "CogGame.h"
#include "ThreadPool.h"
#include "Stopwatch.h"
#include "Component.h"
#include "BaseComponentFactory.h"
#include "BaseComponentFactoryChunk.h"
#include "Entity.h"
#include "EntityFactory.h"
#include <Semaphore.h>
#include "ComponentList.h"

CogGame* CogGame::ourGame;

bool IsInGameThread()
{
	return GetGame().IsInGameThread();
}

CogGame::CogGame()
	: myThreadPool(*new ThreadPool(8)),
	myGameThreadID(ThreadID::Get()),
	myEntityFactory(*new EntityFactory())
{
	// Multiple game instances are not allowed under 1 process
	CHECK(!ourGame);
	ourGame = this;
}

CogGame::~CogGame()
{
	myEntityFactory.ReturnAll();

	for (BaseComponentFactory* factory : myComponentFactories)
		delete factory;
	myComponentFactories.Clear();

	delete &myEntityFactory;

	delete &myThreadPool;

	delete myComponentList;

	ourGame = nullptr;
}

void CogGame::Run()
{
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

		Tick(deltaTime);
	}
}

void CogGame::Tick(const Time& aDeltaTime)
{
	Semaphore doneWithSemaphore;
	Semaphore resume;
	myThreadPool.Pause(resume, doneWithSemaphore);

	DispatchWork(aDeltaTime);

	// Resume the thread pool
	resume.Notify();
	doneWithSemaphore.Wait();
	
	for (;;)
	{
		myThreadPool.Pause(resume, doneWithSemaphore);
	
		auto synchronizedCallback = mySynchronizedCallbacks.Gather();
	
		for (auto synchronizedWork : synchronizedCallback)
			synchronizedWork.TryCall();

		// Resume the thread pool
		resume.Notify();
		doneWithSemaphore.Wait();
	
		// Stop iterating if we did no work
		if (synchronizedCallback.GetLength() == 0)
			break;
	}

	// Println(L"Tick % FPS", 1.f / aDeltaTime.Seconds());
}

void CogGame::DispatchWork(const Time& aDeltaTime)
{
	DispatchTick(aDeltaTime);
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

void CogGame::DispatchTick(const Time& aDeltaTime)
{
	for (BaseComponentFactory* factory : myComponentFactories)
	{
		if (!factory)
			continue;

		factory->IterateChunks([aDeltaTime](BaseComponentFactoryChunk& aChunk)
		{
			aChunk.DispatchTick(aDeltaTime);
		});
	}
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
	return myEntityFactory.Allocate();
}

void CogGame::AssignComponentList(const ComponentList& aComponents)
{
	myComponentList = &aComponents;
}
