#include "pch.h"
#include "Program.h"
#include "Fiber.h"
#include "Stopwatch.h"
#include "Awaitable.h"

Program* gProgram = nullptr;

static thread_local bool gIsCogThread = false;

Program::Program()
	: myMainThread(ThreadID::Get()), mySleepingThreads(0)
{
	gIsCogThread = true;
	
	myNumWorkers = CastBoundsChecked<i32>(std::thread::hardware_concurrency());

	myWorkers.PrepareAdd(myNumWorkers);

	Println(L"Program starting % worker threads...", myNumWorkers);

	for (i32 i = 0; i < myNumWorkers; ++i)
		myWorkers.Emplace(std::thread(&Program::WorkerThread, this));
	
	// Wait for all threads to start sleeping
	std::unique_lock<std::mutex> lck(myWakeMainMutex);
	myWakeMainNotify.wait(lck, [this]() { return myWakeMainFlag; });
	myWakeMainFlag = false;
}

Program::~Program()
{
	Println(L"Program shutting down...");

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

void Program::CheckYieldedFiber(void* aArg)
{
	Fiber& fiber = *static_cast<Fiber*>(aArg);
	Awaitable* awaitable = static_cast<Awaitable*>(fiber.GetYieldedData());

	Program& program = Program::Get();

	if (!awaitable || awaitable->IsReady())
	{
		delete awaitable;
		fiber.Continue();

		// Yielded again
		if (fiber.HasWork())
		{
			program.QueueWork(&Program::CheckYieldedFiber, aArg);
		}
		else
		{
			std::unique_lock<std::mutex> lck(program.myFiberMutex);
			program.myUnusedFibers.Push(&fiber);
		}
	}
	else
	{
		program.QueueWork(&Program::CheckYieldedFiber, aArg);
	}
}

void Program::Run()
{
	Stopwatch watch;
	i32 frames = 0;
	i32 elapsedSeconds = 0;

	Array<QueuedWork> newWork;

	for (;;)
	{
		newWork.Empty();
		myQueuedWork.GatherInto(newWork);

		const i32 numWork = newWork.GetLength();

		// Println(L"numWork = ", numWork);

		if (numWork > 0)
		{
			std::unique_lock<std::mutex> workLck(myWorkMutex);
			
			Swap(myCurrentWorkQueue, newWork);

			const i32 n = Min(numWork, myNumWorkers);
			
			myWakeMainFlag = false;
			
			// PERF: Try switching this to notify_all
			for (i32 i = 0; i < n; ++i)
				myWorkNotify.notify_one();

			{
				// Wait for all work to finish
				std::unique_lock<std::mutex> wakeMainLck(myWakeMainMutex);
				myIsMainRunning = false;
				workLck.unlock();

				myWakeMainNotify.wait(wakeMainLck, [this]() { return myWakeMainFlag; });

				myWakeMainFlag = false;
				myIsMainRunning = true;
			}
		}
		else
		{
			if (myYieldedFibers.GetLength() == 0)
			{
				Println(L"Program finished in % seconds", static_cast<float>(elapsedSeconds) + watch.GetElapsedTime().Seconds());
				break;
			}
		}

		if (myYieldedFibers.GetLength() > 0)
		{
			// TODO: Don't submit as individual elements, instead implement list submit method that does multiple checks per work unit
			for (i32 i = 0; i < myYieldedFibers.GetLength(); ++i)
				QueueWork(&Program::CheckYieldedFiber, myYieldedFibers[i]);

			myYieldedFibers.Resize(0);
		}
		else
		{
			std::this_thread::yield();
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
				WakeMain();
			}

			for (;;)
			{
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

			work.function(work.argument);
		}
	}

	Fiber::YieldExecution();
	// Should not return here once yield is called
	CHECK(false);
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

void Program::WorkerThread()
{
	gIsCogThread = true;
	Fiber::ConvertCurrentThreadToFiber();

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
			std::unique_lock<std::mutex> lck(myWorkMutex);
			myYieldedFibers.Add(fiber);
			fiber = nullptr;
			continue;
		}

		CHECK(myIsStopping);
		break;
	}

	delete fiber;
}

void Program::QueueWork(void(*aFunction)(void*), void* aArgument)
{
	CHECK(gIsCogThread);
	myQueuedWork.Submit({ aFunction, aArgument });
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

void YieldExecution()
{
	Fiber::YieldExecution(nullptr);
}

void Await(Awaitable * aAwaitable)
{
	Fiber::YieldExecution(aAwaitable);
}
