#include "pch.h"
#include "Program.h"
#include "Fiber.h"
#include "Stopwatch.h"
#include "Awaitable.h"
#include "AwaitableSignal.h"
#include "ThreadPool.h"

Program* gProgram = nullptr;

static thread_local bool gIsCogThread = false;

Program::Program()
	: myMainThread(ThreadID::Get()), mySleepingThreads(0)
{
	ThreadID::SetName(String(L"Main Thread"));

	gIsCogThread = true;

	myBackgroundWorkThreadPool = new ThreadPool();

	myNumWorkers = CastBoundsChecked<i32>(std::thread::hardware_concurrency());

	myWorkers.PrepareAdd(myNumWorkers);

	Println(L"Program starting % worker threads...", myNumWorkers);

	myIsMainRunning = false;

	MemoryBarrier();

	for (i32 i = 0; i < myNumWorkers; ++i)
		myWorkers.Emplace(std::thread(&Program::WorkerThread, this, i));

	// Wait for all threads to start sleeping
	std::unique_lock<std::mutex> lck(myWakeMainMutex);
	myWakeMainNotify.wait(lck, [this]() { return myIsMainRunning; });
}

Program::~Program()
{
	Println(L"Program shutting down...");

	delete myBackgroundWorkThreadPool;

	Println(L"Notifying worker threads to stop...");

	{
		std::unique_lock<std::mutex> lck(myWorkMutex);
		myIsStopping = true;
	}

	myWorkNotify.notify_all();

	Println(L"Waiting for worker threads to stop...");

	for (std::thread& thread : myWorkers)
		thread.join();

	Println(L"Finished stopping worker threads!");

	Fiber * fiber;
	while (myUnusedFibers.TryPop(fiber))
		delete fiber;
}

static thread_local bool gContinuingYield = false;

void Program::CheckYieldedFiber(void* aArg)
{
	Fiber& fiber = *static_cast<Fiber*>(aArg);
	CHECK(fiber.HasWork());
	Awaitable* awaitable = static_cast<Awaitable*>(fiber.GetYieldedData());

	Program& program = Program::Get();

	if (!awaitable || awaitable->IsReady())
	{
		// fiber.Continue() will delete this memory
		awaitable = nullptr;

		const bool bPreviousContinuingYield = gContinuingYield;
		gContinuingYield = true;
		fiber.Continue();
		gContinuingYield = bPreviousContinuingYield;

		// Fibers should never finish execution
		CHECK(fiber.HasWork());

		void* yieldedData = fiber.GetYieldedData();

		// TODO: Get rid of magic pointer number
		// 1 == finished yielded work
		if (yieldedData == reinterpret_cast<void*>(1))
		{
			// fiber.SetWork(L"Awaiting more work...");
			std::unique_lock<std::mutex> lck(program.myFiberMutex);
			program.myUnusedFibers.Push(&fiber);
		}
		else if (yieldedData)
		{
			// fiber.SetWork(L"Awaiting another awaitable...");
			program.QueueWork(&Program::CheckYieldedFiber, aArg);
		}
		else
		{
			FATAL(L"Program is exiting, implement here");
		}
	}
	else
	{
		// Not ready yet, todo: merge these checks
		program.QueueWork(&Program::CheckYieldedFiber, aArg);
	}
}

void Program::Run()
{
	Stopwatch watch;
	i32 frames = 0;
	i32 elapsedSeconds = 0;

	Array<QueuedWork> newWork;

	std::unique_lock<std::mutex> workMutexLck(myWorkMutex, std::defer_lock);

	for (;;)
	{
		newWork.Empty();

		workMutexLck.lock();
		myQueuedWork.GatherInto(newWork);

		const i32 numWork = newWork.GetLength();

		if (numWork > 0)
		{
			Swap(myCurrentWorkQueue, newWork);

			const i32 numWorkersToWake = Min(numWork, myNumWorkers);

			if (numWorkersToWake == myNumWorkers)
			{
				myWorkNotify.notify_all();
			}
			else
			{
				// PERF: Try making the switch to notify_all more sensitive
				for (i32 i = 0; i < numWorkersToWake; ++i)
					myWorkNotify.notify_one();
			}

			{
				// Wait for all work to finish
				std::unique_lock<std::mutex> wakeMainLck(myWakeMainMutex);
				myIsMainRunning = false;
				workMutexLck.unlock();

				myWakeMainNotify.wait(wakeMainLck, [this]() { return myIsMainRunning; });

				workMutexLck.lock();
			}
		}
		else if (!AwaitableSignal::IsWaitingOnAnySignals())
		{
			if (myYieldedFibers.GetLength() == 0)
			{
				Println(L"Program finished in % seconds", static_cast<float>(elapsedSeconds) + watch.GetElapsedTime().Seconds());
				break;
			}
		}

		Array<Fiber*> yieldedFibers = Move(myYieldedFibers);
		myYieldedFibers = Array<Fiber*>();
		workMutexLck.unlock();

		Array<Fiber*> readyFibers;
		const i32 gatheredSignals = AwaitableSignal::GatherSignaledFibers(readyFibers);

		// Println(L"Gathered % signals", gatheredSignals);

		yieldedFibers.Append(readyFibers);

		// Yield if we're not currently working
		if (yieldedFibers.GetLength() == 0)
			std::this_thread::yield();

		if (yieldedFibers.GetLength() > 0)
		{
			// Distribute work-amounts "equally"
			yieldedFibers.Shuffle();

			// Println(L"% awaitables", yieldedFibers.GetLength());

			// TODO: Don't submit as individual elements, instead implement list submit method that does multiple checks per work unit
			for (i32 i = 0; i < yieldedFibers.GetLength(); ++i)
				QueueWork(&Program::CheckYieldedFiber, yieldedFibers[i]);

			yieldedFibers.Resize(0);
		}

		++frames;

		if (watch.GetElapsedTime().Seconds() > 1.f)
		{
			watch.Restart();

			Println(L"% FPS", frames);
			frames = 0;

			++elapsedSeconds;
		}
	}
}

Program& Program::Create()
{
	CHECK(gProgram == nullptr);
	gProgram = new Program();
	return *gProgram;
}

void Program::FiberMain()
{
	while (!myIsStopping)
	{
		std::unique_lock<std::mutex> lck(myWorkMutex);

		if (myCurrentWorkQueue.GetLength() == 0)
		{
			++mySleepingThreads;

			if (mySleepingThreads == myNumWorkers)
			{
				{
					// Could probably be changed to a memory barrier
					std::unique_lock<std::mutex> mainLock(myWakeMainMutex);
					myIsMainRunning = true;
				}

				myWakeMainNotify.notify_one();
			}

			for (;;)
			{
				// Fiber::SetCurrentWork(L"Sleeping (no work/waiting for main)...");
				myWorkNotify.wait(lck);

				if (myIsStopping)
					break;

				if (myIsMainRunning)
					continue;

				if (myCurrentWorkQueue.GetLength() == 0)
					continue;

				break;
			}

			--mySleepingThreads;
		}

		if (!myIsStopping)
		{
			const auto work = myCurrentWorkQueue.RemoveAt(0);

			lck.unlock();

			// Fiber::SetCurrentWork(L"Working...");

			work.function(work.argument);

			// Println(L"Returning from work...");

			if (gContinuingYield)
			{
				// Println(L"Yielding after work...");
				// TODO: Get rid of magic pointer number
				Fiber::YieldExecution(reinterpret_cast<void*>(1));
			}
		}
	}

	Fiber::YieldExecution(nullptr);
	// Should not return here once yield is called
	CHECK(false);
}

void Program::WorkerThread(const i32 aThreadIndex)
{
	ThreadID::SetName(Format(L"Worker Thread %", aThreadIndex));

	gIsCogThread = true;
	Fiber::ConvertCurrentThreadToFiber(L"Program Worker Thread");

	Fiber* fiber = nullptr;

	for (;;)
	{
		if (fiber == nullptr)
		{
			std::unique_lock<std::mutex> lck(myFiberMutex);

			if (!myUnusedFibers.TryPop(fiber))
				fiber = nullptr;
		}

		if (fiber == nullptr)
			fiber = new Fiber();

		fiber->Execute([](void* aArg)
			{
				Program& program = *static_cast<Program*>(aArg);
				program.FiberMain();

			}, this);

		// Work yielded
		if (fiber->HasWork())
		{
			CHECK(fiber->GetYieldedData());
			// TODO: Fix this pedantic check
			// CHECK(dynamic_cast<Awaitable*>(fiber->GetYieldedData()));
			Awaitable& awaitable = *static_cast<Awaitable*>(fiber->GetYieldedData());

			if (awaitable.UsePolling())
			{
				std::unique_lock<std::mutex> lck(myWorkMutex);
				myYieldedFibers.Add(fiber);
			}
			
			fiber = nullptr;
			continue;
		}

		CHECK(myIsStopping);
		break;
	}

	delete fiber;
}

bool Program::IsInManagedThread() const
{
	return gIsCogThread;
}

void Program::QueueWork(void(*aFunction)(void*), void* aArgument)
{
	if (IsInManagedThread())
	{
		myQueuedWork.Submit({ aFunction, aArgument });
	}
	else
	{
		std::unique_lock<std::mutex> lck(myWorkMutex);
		myQueuedWork.Submit({ aFunction, aArgument });
	}
}

void Program::QueueBackgroundWork(void(*aFunction)(void*), void* aArgument)
{
	myBackgroundWorkThreadPool->QueueSingle([aFunction, aArgument]()
		{
			aFunction(aArgument);
		});
}

void* Program::AllocateRaw(TypeID<void> aTypeID, BaseFactory & (*aFactoryAllocator)())
{
	BaseFactory*& allocator = myAllocators.FindOrAdd(aTypeID.GetUnderlyingInteger());

	if (!allocator)
		allocator = &aFactoryAllocator();

	return allocator->AllocateRawObject();
}

void Program::Return(TypeID<void> aTypeID, void* aObject)
{
	if (BaseFactory * *allocatorPtrPtr = myAllocators.Find(aTypeID.GetUnderlyingInteger()))
	{
		if (BaseFactory * allocator = *allocatorPtrPtr)
		{
			allocator->ReturnRawObject(aObject);
			return;
		}
	}

	FATAL(L"Tried to a return a object to a removed object allocator.");
}

void Await(Awaitable * aAwaitable)
{
	// Should call Fiber::YieldExecution(this) internally
	aAwaitable->StartWaiting();
	CHECK(aAwaitable->IsReady());
}
