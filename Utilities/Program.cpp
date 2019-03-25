#include "pch.h"
#include "Program.h"
#include "Fiber.h"
#include "Stopwatch.h"
#include "Awaitable.h"
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

	// Wait fo rall threads to start sleeping
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

	while (myQueuedFibers.TryPop(fiber))
		delete fiber;
}

void Program::Run()
{
	Stopwatch watch;
	i32 frames = 0;
	i32 elapsedSeconds = 0;

	std::unique_lock<std::mutex> workMutexLck(myWorkMutex, std::defer_lock);

	for (;;)
	{
		workMutexLck.lock();

		const i32 numWork = myWorkQueue.GetLength() + myQueuedFibers.GetLength();

		if (numWork > 0)
		{
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

			// Wait for all work to finish
			std::unique_lock<std::mutex> wakeMainLck(myWakeMainMutex);
			myIsMainRunning = false;
			workMutexLck.unlock();

			myWakeMainNotify.wait(wakeMainLck, [this]() { return myIsMainRunning; });

			workMutexLck.lock();

			// std::this_thread::yield();
		}
		else
		{
			Println(L"Program finished in % seconds", static_cast<float>(elapsedSeconds) + watch.GetElapsedTime().Seconds());
			break;
		}

		workMutexLck.unlock();

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
		
		if (myQueuedFibers.GetLength() > 0)
		{
			Fiber* fiberToExecute = myQueuedFibers.Pop();

			lck.unlock();

			Fiber* currentFiber = Fiber::GetCurrentlyExecutingFiber();
			
			// FIXME Register currentFiber as unused
			

			FiberHandle workerThreadFiber = currentFiber->GetCallingFiber();
			
			fiberToExecute->Continue(&workerThreadFiber);
			continue;
		}
		
		if (myWorkQueue.GetLength() > 0)
		{
			const auto work = myWorkQueue.RemoveAt(0);

			lck.unlock();

			// Fiber::SetCurrentWork(L"Working...");

			work.function(work.argument);

			// Println(L"Returning from work...");
			continue;
		}

		if (myWorkQueue.GetLength() == 0 && myQueuedFibers.GetLength() == 0)
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

				if (myWorkQueue.GetLength() == 0 && myQueuedFibers.GetLength() == 0)
					continue;

				break;
			}

			--mySleepingThreads;
		}
	}

	Fiber::YieldExecution(nullptr);
	// Should not return here once yield is called
	CHECK(false);
}

void Program::WorkerThread(const i32 aThreadIndex)
{
	auto setAffinityResult = SetThreadAffinityMask(GetCurrentThread(), 1 < aThreadIndex);
	CHECK(setAffinityResult != -1);

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
		{
			fiber = new Fiber();

			fiber->Execute([](void* aArg)
				{
					Program& program = *static_cast<Program*>(aArg);
					program.FiberMain();

				}, this);
		}
		else
		{
			fiber->Continue();
		}

		// Work yielded
		if (fiber->HasWork())
		{
			if (void* yieldedData = fiber->RetrieveYieldedData())
			{
				std::atomic_bool& flagToSet = *static_cast<std::atomic_bool*>(yieldedData);
				flagToSet.store(true);
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
	std::unique_lock<std::mutex> lck(myWorkMutex);
	myWorkQueue.Add({ aFunction, aArgument });
	lck.unlock();

	myWorkNotify.notify_one();
}

void Program::QueueFiber(Fiber * aFiber)
{
	std::unique_lock<std::mutex> lck(myWorkMutex);
	myQueuedFibers.Add(aFiber);
	lck.unlock();

	myWorkNotify.notify_one();
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
