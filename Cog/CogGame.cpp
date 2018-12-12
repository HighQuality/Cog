#include "pch.h"
#include "CogGame.h"
#include "ThreadPool.h"
#include "Stopwatch.h"
#include "BinaryData.h"
#include "CogGameWorld.h"
#include <Semaphore.h>

CogGame* CogGame::ourGame;

bool IsInGameThread()
{
	return CogGame::Get().IsInGameThread();
}

CogGame::CogGame()
	: myThreadPool(*new ThreadPool(8)),
	myGameThreadID(ThreadID::Get())
{
	// Multiple game instances are not allowed under 1 process
	CHECK(!ourGame);
	ourGame = this;
}

CogGame::~CogGame()
{
	delete &myThreadPool;

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

	for (auto world : myWorlds)
		world->DispatchTick(aDeltaTime);
	
	// Resume the thread pool
	resume.Notify();
	doneWithSemaphore.Wait();
	
	for (;;)
	{
		myThreadPool.Pause(resume, doneWithSemaphore);
	
		auto synchronizedCallback = mySynchronizedCallbacks.Gather();
	
		for (auto synchronizedWork : synchronizedCallback)
		{
			synchronizedWork->Call();
			delete synchronizedWork;
		}

		// Resume the thread pool
		resume.Notify();
		doneWithSemaphore.Wait();
	
		// Stop iterating if we did no work
		if (synchronizedCallback.GetLength() == 0)
			break;
	}

	// Println(L"Tick % FPS", 1.f / aDeltaTime.Seconds());
}

void CogGame::AddWorld(CogGameWorld& aWorld)
{
	CHECK(IsInGameThread());
	myWorlds.Add(&aWorld);
}
