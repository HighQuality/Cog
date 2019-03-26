#include "pch.h"
#include "Program.h"
#include "Fiber.h"
#include "Stopwatch.h"
#include "Awaitable.h"
#include "ThreadPool.h"

Program* gProgram = nullptr;

static thread_local bool gIsCogThread = false;

static std::atomic<i32> gNextFiberIndex = 0;

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
			std::unique_lock<std::mutex> fibersLck(myFiberMutex);
			// TODO: Make this check "better"?
			// Basically, if the number of unused fibers added to the number of workers equal the next fiber index we have no live fibers waiting on something
			// Each worker thread always have exactly one fiber allocated to itself
			if (myUnusedFibers.GetLength() + myNumWorkers == gNextFiberIndex)
			{
				Println(L"Program finished in %ms", (static_cast<float>(elapsedSeconds) + watch.GetElapsedTime().Seconds()) * 1000.f);
				break;
			}
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

	Println(L"Program finished with % fibers allocated", gNextFiberIndex.load());
}

Program& Program::Create()
{
	CHECK(gProgram == nullptr);
	gProgram = new Program();
	return *gProgram;
}

void Program::FiberMain()
{
	Fiber* currentFiber = Fiber::GetCurrentlyExecutingFiber();

	std::unique_lock<std::mutex> lck(myWorkMutex);

	while (!myIsStopping)
	{
		if (myQueuedFibers.GetLength() > 0)
		{
			Fiber* fiberToExecute = myQueuedFibers.Pop();
			lck.unlock();

			FiberResumeData resumeData(FiberResumeType::ResumeFromAwait);
			resumeData.resumeFromAwaitData.sleepingFiber = currentFiber;
			fiberToExecute->Resume(resumeData);

			lck.lock();
			continue;
		}

		if (myWorkQueue.GetLength() > 0)
		{
			QueuedWork work = myWorkQueue.RemoveAt(0);
			lck.unlock();

			work.function(work.argument);

			lck.lock();
			continue;
		}

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

	relock:
		myWorkNotify.wait(lck);

		if (myIsMainRunning)
			goto relock;

		--mySleepingThreads;
	}
}

void Program::WorkerThread(const i32 aThreadIndex)
{
	// auto setAffinityResult = SetThreadAffinityMask(GetCurrentThread(), 1 << aThreadIndex);
	// CHECK(setAffinityResult != -1);

	ThreadID::SetName(Format(L"Worker Thread %", aThreadIndex));

	gIsCogThread = true;
	Fiber::ConvertCurrentThreadToFiber(Format(L"Program Worker Thread ", aThreadIndex).View());

	Fiber* fiber = nullptr;

	for (;;)
	{
		if (fiber == nullptr)
			fiber = GetUnusedFiber();

		// Resume the fiber
		FiberResumeData returnedData = fiber->Resume(FiberResumeData(FiberResumeType::Starting));

		switch (returnedData.type)
		{
		case FiberResumeType::Await:
		{
			// TODO: Queue in one lock
			for (Awaitable* awaitable : *returnedData.awaitData.workItems)
				awaitable->StartWork();
		} break;

		case FiberResumeType::Exiting:
			CHECK(myIsStopping);
			delete fiber;
			return;

		default:
			FATAL(L"FiberResumeType out of range (", static_cast<i32>(returnedData.type), L")");
		}

		fiber = nullptr;
	}
}

bool Program::IsInManagedThread() const
{
	return gIsCogThread;
}

void Program::RegisterUnusedFiber(Fiber* aFiber)
{
	// Println(L"Registering fiber % as unused", aFiber->GetName());

	std::unique_lock<std::mutex> lck(myFiberMutex);
	myUnusedFibers.Push(aFiber);
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

Fiber* Program::GetUnusedFiber()
{
	{
		std::unique_lock<std::mutex> lck(myFiberMutex);

		Fiber* fiber;
		if (myUnusedFibers.TryPop(fiber))
			return fiber;
	}

	const i32 newFiberIndex = gNextFiberIndex.fetch_add(1);

	// Println(L"Allocating fiber #", newFiberIndex + 1);

	Fiber* newFiber = new Fiber(
		Format(L"Fiber %", newFiberIndex).View(),
		[](void* aArg)
	{
		Program& program = *static_cast<Program*>(aArg);
		program.FiberMain();

	}, this);

	return newFiber;
}
