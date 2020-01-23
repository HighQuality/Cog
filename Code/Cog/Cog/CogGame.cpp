#include "CogPch.h"
#include "CogGame.h"

#include <Time/Stopwatch.h>
#include <Cog/Threading/Fibers/Await.h>
#include "Program.h"

#include "ResourceManager.h"
#include "MessageSystem.h"

bool IsInGameThread()
{
	return GetGame().IsInGameThread();
}

void RegisterTypeList();

void RunGame()
{
	RegisterTypeList();

	Program::Create();

	Ptr<CogGame> game = NewObject<CogGame>();
	game->Run();

	game->ReturnToAllocator();

	Program::Destroy();
}

CogGame::CogGame()
	: myMainThreadID(ThreadID::Get()),
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
	myResourceManager->Destroy();

	for (i32 i = 0; i < myScheduledDestroys.GetLength(); ++i)
		TickDestroys();

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

bool CogGame::IsInGameThread() const
{
	return myMainThreadID == ThreadID::Get();
}

void CogGame::SynchronizedTick(const Time& aDeltaTime)
{
	QueueDispatchers(aDeltaTime);

	// Execute this frame's work
	gProgram->Run(false);

	while (myMessageSystem->PostMessages())
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

void CogGame::DispatchTick()
{
}

void CogGame::CreateResourceManager()
{
	myResourceManager = NewObject<ResourceManager>();
}

void CogGame::UpdateFrameData(FrameData& aData, const Time& aDeltaTime)
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
	std::unique_lock<std::mutex> lck(myDestroyMutex);
	myScheduledDestroys.Last().Add(&aObject);
}
