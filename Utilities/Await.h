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
		mySleepingFiber = Fiber::GetCurrentlyExecutingFiber();

		MemoryBarrier();

		// TODO: Queue in one lock
		for (i32 i = 0; i < myAwaitables.GetLength(); ++i)
			myAwaitables[i]->StartWork();

		// Put this fiber to sleep until we're woken up
		Fiber::YieldExecution(&myIsWaiting);

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
			// Spin lock until the fiber has stopped executing
			while (!myIsWaiting.load())
			{ }
			
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
	std::atomic_bool myIsWaiting;
	Array<Awaitable*> myAwaitables;
	Await* myFollowedBy = nullptr;
	Fiber* mySleepingFiber = nullptr;

	bool myWasMovedOrExecuted = false;
};
