#pragma once
#include "Fiber.h"
#include "Program.h"

class Awaitable;

class Await
{
public:
	template <typename ...TArgs>
	explicit Await(TArgs& ...aAwaitables)
	{
		myCounter.store(sizeof...(aAwaitables));
		myAwaitables = { &aAwaitables... };

		for (Awaitable* awaitable : myAwaitables)
			awaitable->myAwaiter = this;
	}

	void Execute()
	{
		// No work
		if (myAwaitables.GetLength() == 0)
			return;

		mySleepingFiber = Fiber::GetCurrentlyExecutingFiber();

		MemoryBarrier();

		for (Awaitable* awaitable : myAwaitables)
			awaitable->myAwaiter = this;

		// Put this fiber to sleep until we're woken up
		FiberResumeData resumeData;
		resumeData.type = FiberResumeType::Await;
		resumeData.awaitData.workItems = &myAwaitables;
		FiberResumeData returnedData = UtilitiesTLS::GetThisThreadsStartingFiber()->Resume(resumeData);

		CHECK(returnedData.type == FiberResumeType::ResumeFromAwait);
		CHECK(returnedData.resumeFromAwaitData.sleepingFiber);

		Program::Get().RegisterUnusedFiber(returnedData.resumeFromAwaitData.sleepingFiber);

		// Checks that the counter is 0 and prevent other threads from simultaneously succeeding this check
		CHECK(myCounter.fetch_sub(1) == 0);

		mySleepingFiber = nullptr;

		if (myFollowedBy)
			myFollowedBy->Execute();

		myWasMovedOrExecuted = true;
	}

	template <typename ...TArgs>
	void Then(TArgs& ...aAwaitables)
	{
		CHECK(myFollowedBy == nullptr);
		myFollowedBy = new Await(aAwaitables...);
	}

	void DecrementCounter()
	{
		if (myCounter.fetch_sub(1) == 1)
		{
			Program::Get().QueueFiber(mySleepingFiber);
		}
	}

	~Await()
	{
		if (!myWasMovedOrExecuted)
			Execute();

		delete myFollowedBy;
		myFollowedBy = nullptr;
	}

private:
	std::atomic<u8> myCounter;
	Array<Awaitable*> myAwaitables;
	Await* myFollowedBy = nullptr;
	Fiber* mySleepingFiber = nullptr;

	bool myWasMovedOrExecuted = false;
};
