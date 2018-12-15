#include "pch.h"
#include "CogGame.h"
#include "ThreadPool.h"
#include "Stopwatch.h"
#include "Component.h"
#include "BaseComponentFactory.h"
#include "BaseComponentFactoryChunk.h"
#include "Object.h"
#include "ObjectFactory.h"
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
	myObjectFactory(*new ObjectFactory())
{
	// Multiple game instances are not allowed under 1 process
	CHECK(!ourGame);
	ourGame = this;
}

CogGame::~CogGame()
{
	myObjectFactory.ReturnAll();

	for (BaseComponentFactory* factory : myComponentFactories)
		delete factory;
	myComponentFactories.Clear();

	delete &myObjectFactory;

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

ObjectInitializer CogGame::CreateObject()
{
	CHECK(IsInGameThread());
	return ObjectInitializer(AllocateObject());
}

Object& CogGame::AllocateObject()
{
	CHECK(IsInGameThread());
	return myObjectFactory.Allocate();
}

void CogGame::AssignComponentList(const ComponentList& aComponents)
{
	myComponentList = &aComponents;
}
