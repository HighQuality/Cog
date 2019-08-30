#include "pch.h"
#include "CogGame.h"

#include <Time/Stopwatch.h>
#include <Threading/Fibers/Await.h>
#include "Program.h"

#include "TypeList.h"
#include "ResourceManager.h"
#include "MessageSystem.h"

CogGame* CogGame::ourGame;

bool IsInGameThread()
{
	return GetGame().IsInGameThread();
}

CogGame::CogGame()
	: myGameThreadID(ThreadID::Get()),
	myFrameData(MakeUnique<FrameData>()),
	myMessageSystem(MakeUnique<MessageSystem>())
{
	// Multiple game instances are not allowed under 1 process
	CHECK(!ourGame);
	ourGame = this;

	// Destroyed objects are scheduled for the entirety of the current and next frame before being destroyed
	myScheduledDestroys.Resize(2);
}

CogGame::~CogGame()
{
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

	while (myMessageSystem->PostMessages())
		gProgram->Run(false);
}

void CogGame::QueueDispatchers(const Time & aDeltaTime)
{
	UpdateFrameData(*myFrameData, aDeltaTime);

	gProgram->QueueHighPrioWork<CogGame>([](CogGame * aGame)
		{
			aGame->DispatchTick();
		}, this);
}

void CogGame::DispatchTick()
{
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

void CogGame::CreateResourceManager()
{
	myResourceManager = CreateObject<ResourceManager>();
}

void CogGame::AssignTypeList(UniquePtr<const TypeList> aTypeList)
{
	myTypeList = Move(aTypeList);
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
