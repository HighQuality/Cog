#include "pch.h"
#include "Program.h"

Program gProgram(ThreadID::Get());

Program::Program(const ThreadID& aThreadID)
	: myMainThread(aThreadID), mySleepingThreads(0)
{
	memset(myWorkerThreadIds, 0, sizeof myWorkerThreadIds);
	memset(myWorkerYieldDepth, 0, sizeof myWorkerYieldDepth);

	myNumWorkers = CastBoundsChecked<i32>(std::thread::hardware_concurrency());

	myWorkers.PrepareAdd(myNumWorkers);

	Println(L"Program starting {0} worker threads...", myNumWorkers);

	for (i32 i = 0; i < myNumWorkers; ++i)
	{
		myWorkerThreadIds[i] = 0;
		myWorkerYieldDepth[i] = 0;
		myWorkers.Emplace(std::thread(&Program::WorkerThread, this, i));
	}

	Println(L"Waiting for all threads to wake up...");

	for (i32 i = 0; i < myNumWorkers; ++i)
	{
		while (myWorkerThreadIds[i] == 0)
			std::this_thread::yield();
	}

	Println(L"Main thread ID is {0}", aThreadID.GetInteger());
}

Program::~Program()
{
	Println(L"Program shutting down...");

	Println(L"Notifying worker threads to stop...");

	{
		std::unique_lock<std::mutex> lck(myWorkMutex);
		myIsStopping = true;
	}

	for (i32 i = 0; i < myNumWorkers; ++i)
		myWorkNotify[i].notify_one();

	Println(L"Waiting for worker threads to stop...");

	for (std::thread& thread : myWorkers)
		thread.join();

	Println(L"Finished stopping worker threads!");
}

void Program::Run()
{
	for (;;)
	{
		auto newWork = myQueuedWork.Gather();

		const i32 numWork = newWork.GetLength();

		if (numWork > 0)
		{
			{
				std::unique_lock<std::mutex> lck(myWorkMutex);
				myExecutionIsSynchronized = false;
				myCurrentWorkQueue = Move(newWork);
			}

			const i32 n = Min(numWork, myNumWorkers);

			for (i32 i = 0; i < n; ++i)
				myWorkNotify[i].notify_one();

			{
				// Wait for all work to finish
				std::unique_lock<std::mutex> lck(myWakeMainMutex);
				myWakeMainNotify.wait(lck, [this]() { return myWakeMainFlag; });
				myWakeMainFlag = false;
			}

			myExecutionIsSynchronized = true;
		}
		else
		{
			// No work this frame, return here and end program?
			std::this_thread::yield();
		}

		// Finish tasks that are yielding and waiting for the main thread
		for (i32 i = 0; i < myNumWorkers; ++i)
		{
			while (myWorkerYieldDepth[i] > 0)
			{
				/*
				 * TODO:
				 * When we wake up a thread it will execute it's entire yield history before giving back control to the main thread
				 * This is even though not all actions are required to be executed synchronized, most likely wasting a lot of time
				 */

				std::unique_lock<std::mutex> lck(myWakeMainMutex);
				myWorkNotify[i].notify_one();

				// Wait for the synchronized work to finish on this thread
				myWakeMainNotify.wait(lck, [this]() { return myWakeMainFlag; });
				myWakeMainFlag = false;
			}
		}
	}
}

void Program::WorkerThread(const i32 aWorkerId)
{
	const ThreadID& threadId = ThreadID::Get();
	myWorkerThreadIds[aWorkerId] = threadId.GetInteger();

	while (!myIsStopping)
	{
		YieldExecution(threadId);

		// TODO: Is this actually okay to read here? The only place it could possibly be written to is when all worker threads are sleeping and the main thread is working
		// If we're running in synchronized mode, let the main thread know we're done
		if (IsExecutionSynchronized())
			WakeMain();
	}
}

constexpr i32 MaxYieldDepth = 128;

void Program::YieldExecution(const ThreadID& aThreadId)
{
	CHECK(aThreadId != myMainThread);

	u8 & yieldDepth = myWorkerYieldDepth[aThreadId.GetInteger()];
	const u8 previousYieldDepth = yieldDepth;
	yieldDepth++;

	// TODO: Switch this mechanic to actually measure the free space on the stack instead of the yield depth
	if (!ENSURE(yieldDepth < MaxYieldDepth))
	{
		// TODO: Implement idle threads taking over threads with too much yield depth
		FATAL(L"Program fallback idle threads not implemented (went over max yield depth {0})", MaxYieldDepth);
	}

	std::unique_lock<std::mutex> lck(myWorkMutex);

	if (myCurrentWorkQueue.GetLength() == 0)
	{
		++mySleepingThreads;

		if (mySleepingThreads == myNumWorkers)
		{
			WakeMain();
		}

		for (;;)
		{
			myWorkNotify[aThreadId.GetInteger()].wait(lck);
			--mySleepingThreads;

			if (IsExecutionSynchronized())
				break;

			if (myIsStopping)
				break;

			if (myCurrentWorkQueue.GetLength() == 0)
				continue;

			break;
		}
	}

	if (!IsExecutionSynchronized())
	{
		if (!myIsStopping)
		{
			void(*currentWork)(void*) = myCurrentWorkQueue.RemoveAt(0);

			lck.unlock();

			void* userData = nullptr;
			currentWork(userData);
		}
	}

	// Don't return before doing this
	--yieldDepth;
	// Yield Depth was not calculated properly (paths that don't decrement yieldDepth in this function?)
	CHECK(yieldDepth == previousYieldDepth);
}

void Program::WakeMain()
{
	{
		// Could probably be changed to a memory barrier
		std::unique_lock<std::mutex> mainLock(myWakeMainMutex);
		myWakeMainFlag = true;
	}

	myWakeMainNotify.notify_one();
}

void* Program::Allocate(TypeID<void> aTypeID, BaseProgramAllocator & (*aFactoryAllocator)())
{
	while (!IsExecutionSynchronized())
		YieldExecution();

	BaseProgramAllocator*& allocator = myAllocators.FindOrAdd(aTypeID.GetUnderlyingInteger());

	if (!allocator)
		allocator = &aFactoryAllocator();

	return allocator->AllocateRawObject();
}

void Program::Return(TypeID<void> aTypeID, void* aObject)
{
	while (!IsExecutionSynchronized())
		YieldExecution();

	if (BaseProgramAllocator * *allocatorPtrPtr = myAllocators.Find(aTypeID.GetUnderlyingInteger()))
	{
		if (BaseProgramAllocator * allocator = *allocatorPtrPtr)
		{
			allocator->ReturnRawObject(aObject);
			return;
		}
	}

	FATAL(L"Tried to a return a object to a removed object allocator.");
}

void YieldExecution()
{
	Program::Get().YieldExecution();
}
