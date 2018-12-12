#include "pch.h"
#include "CogGame.h"
#include "ThreadPool.h"
#include "Stopwatch.h"
#include "BinaryData.h"
#include "CogGameWorld.h"

CogGame::CogGame()
{
	CHECK(gThreadID == 0 || gThreadID == 1);
	gThreadID = 1;

	BinaryData b;
	b.Compress();

	myThreadPool = new ThreadPool();
}

CogGame::~CogGame()
{
	delete myThreadPool;
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
	for (auto world : myWorlds)
		world->DispatchTick(aDeltaTime);

	for (;;)
	{
		// Wait for work to finish
		myThreadPool->Barrier();


	}
}

void CogGame::AddWorld(CogGameWorld& aWorld)
{
	myWorlds.Add(&aWorld);
}

