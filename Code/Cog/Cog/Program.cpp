#include "CogPch.h"
#include "Program.h"
#include "Threading/Fibers/Fiber.h"
#include "Threading/Fibers/Awaitable.h"
#include "TypeData.h"
#include <Threading\ThreadNotify.h>

static std::atomic<i32> gNextFiberIndex = 0;

bool Program::Starting()
{
	if (!Base::Starting())
		return false;

	GetObjectPool().SetProgramContext(GetProgramContext());

	// Destroyed objects are scheduled for the entirety of the current and next frame before being destroyed
	GetScheduledDestroys().Resize(2);

	Println(NewObject<TestType>(nullptr)->GetType().GetName());

	SetMessageSystem(NewObject<MessageSystem>(nullptr));

	ThreadID::SetName(String(L"Main Thread"));
	SetMainThreadID(&ThreadID::Get());

	CogTLS::MarkThreadAsManaged();

	GetBackgroundWorkThreadPool() = MakeUnique<ThreadPool>();

	const auto numWorkers = SetNumWorkers(CastBoundsChecked<i32>(std::thread::hardware_concurrency()));

	GetWorkers().PrepareAdd(numWorkers);

	Println(L"Program starting % worker threads...", numWorkers);

	SetIsMainRunning(false);

	MemoryBarrier();

	for (i32 i = 0; i < numWorkers; ++i)
		GetWorkers().Emplace(std::thread(&Program::WorkerThread, this, i));

	// Wait for all threads to start sleeping
	std::unique_lock<std::mutex> lck(GetWakeMainMutex());

	while (!IsMainRunning())
		GetWakeMainNotify().wait(lck);

	return true;
}

void Program::ShuttingDown()
{
 	Println(L"Program shutting down...");

	for (i32 i = 0; i < GetScheduledDestroys().GetLength(); ++i)
		TickDestroys();

	GetBackgroundWorkThreadPool().Clear();

	Println(L"Notifying worker threads to stop...");

	scoped_lock (GetWorkMutex())
	{
		SetIsStopping(true);
		// Worker threads don't run while this is true
		SetIsMainRunning(false);
	}

	GetWorkNotify().notify_all();

	Println(L"Waiting for worker threads to stop...");

	for (std::thread& thread : GetWorkers())
		thread.join();

	SetIsMainRunning(true);

	Println(L"Finished stopping worker threads!");

	Fiber* fiber;
	while (GetUnusedFibers().TryPop(fiber))
		delete fiber;

	while (GetQueuedFibers().TryPop(fiber))
		delete fiber;

	Base::ShuttingDown();
}

void Program::Run()
{
	Stopwatch watch;
	bool isFirstFrame = true;

	while (ShouldKeepRunning())
	{
		f32 deltaSeconds;

		if (isFirstFrame)
		{
			deltaSeconds = 1.f / 60.f;
			isFirstFrame = false;
		}
		else
		{
			deltaSeconds = watch.GetElapsedTime().GetTotalSeconds();
		}

		watch.Restart();

		SynchronizedTick(deltaSeconds);

		// Execute this frame's work
		Step(false);

		while (GetMessageSystem()->PostMessages())
			Step(false);

		TickDestroys();
	}
}

void Program::SynchronizedTick(const f32 aDeltaSeconds)
{
}

void Program::Step(const bool aPrintDebugInfo)
{
	std::unique_lock<std::mutex> workMutexLck(GetWorkMutex(), std::defer_lock);

	for (;;)
	{
		workMutexLck.lock();

		const i32 numWork = GetWorkQueue().GetLength() + GetHighPrioWorkQueue().GetLength() + GetQueuedFibers().GetLength();

		if (numWork > 0)
		{
			const i32 numWorkersToWake = Min(numWork, GetNumWorkers());

			if (numWorkersToWake == GetNumWorkers())
			{
				GetWorkNotify().notify_all();
			}
			else
			{
				// PERF: Try making the switch to notify_all more sensitive
				for (i32 i = 0; i < numWorkersToWake; ++i)
					GetWorkNotify().notify_one();
			}

			// Wait for all work to finish
			std::unique_lock<std::mutex> wakeMainLck(GetWakeMainMutex());
			SetIsMainRunning(false);
			workMutexLck.unlock();

			GetWakeMainNotify().wait(wakeMainLck, [this]() { return IsMainRunning(); });

			workMutexLck.lock();

			// std::this_thread::yield();
		}
		else
		{
			scoped_lock (GetFiberMutex())
			{
				// TODO: Make this check "better"?
				// Basically, if the number of unused fibers added to the number of workers equal the next fiber index we have no live fibers waiting on something
				// Each worker thread always have exactly one fiber allocated to itself
				if (GetUnusedFibers().GetLength() + GetNumWorkers() == gNextFiberIndex)
				{
					if (aPrintDebugInfo)
						Println(L"Program finished in %ms", (static_cast<f32>(GetElapsedSeconds()) + GetWatch().GetElapsedTime().GetTotalSeconds()) * 1000.f);
					break;
				}
			}
		}

		workMutexLck.unlock();

		SetFramesThisSecond(GetFramesThisSecond() + 1);

		if (GetWatch().GetElapsedTime().GetTotalSeconds() > 1.f)
		{
			GetWatch().Restart();

			Println(L"% FPS", GetFramesThisSecond());
			SetFramesThisSecond(0);

			SetElapsedSeconds(GetElapsedSeconds() + 1);
		}
	}

	if (aPrintDebugInfo)
		Println(L"Program finished with % fibers allocated", gNextFiberIndex.load());
}

void Program::FiberMain()
{
	Fiber* currentFiber = Fiber::GetCurrentlyExecutingFiber();

	{
		std::unique_lock<std::mutex> lck(GetWorkMutex());

		while (!IsStopping())
		{
			if (GetHighPrioWorkQueue().GetLength() > 0)
			{
				QueuedProgramWork work = GetHighPrioWorkQueue().RemoveAt(0);
				lck.unlock();

				work.function(work.argument);

				lck.lock();
				continue;
			}

			if (GetQueuedFibers().GetLength() > 0)
			{
				Fiber* fiberToExecute = GetQueuedFibers().Pop();
				lck.unlock();

				FiberResumeData resumeData(FiberResumeType::ResumeFromAwait);
				resumeData.resumeFromAwaitData.sleepingFiber = currentFiber;
				fiberToExecute->Resume(resumeData);

				lck.lock();
				continue;
			}

			if (GetWorkQueue().GetLength() > 0)
			{
				QueuedProgramWork work = GetWorkQueue().RemoveAt(0);
				lck.unlock();

				work.function(work.argument);

				lck.lock();
				continue;
			}

			SetSleepingThreads(GetSleepingThreads() + 1);

			if (GetSleepingThreads() == GetNumWorkers())
			{
				{
					// Could probably be changed to a memory barrier
					scoped_lock (GetWakeMainMutex())
						SetIsMainRunning(true);
				}

				GetWakeMainNotify().notify_one();
			}

			do
			{
				GetWorkNotify().wait(lck);

			} while (IsMainRunning());

			SetSleepingThreads(GetSleepingThreads() - 1);
		}
	}

	CogTLS::GetThisThreadsStartingFiber()->Resume(FiberResumeData(FiberResumeType::Exiting));
}

void Program::WorkerThread(const i32 aThreadIndex)
{
	// auto setAffinityResult = SetThreadAffinityMask(GetCurrentThread(), 1 << aThreadIndex);
	// CHECK(setAffinityResult != -1);

	ThreadID::SetName(Format(L"Worker Thread %", aThreadIndex));

	CogTLS::MarkThreadAsManaged();
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
			for (AwaitableBase* awaitable : *returnedData.awaitData.workItems)
				awaitable->TriggerWork();
		} break;

		case FiberResumeType::Exiting:
			CHECK(IsStopping());
			delete fiber;
			return;

		default:
			FATAL(L"FiberResumeType out of range (", static_cast<i32>(returnedData.type), L")");
		}

		fiber = nullptr;
	}
}

bool Program::IsInMainThread() const
{
	return *GetMainThreadID() == ThreadID::Get();
}

bool Program::IsInManagedThread() const
{
	return CogTLS::IsInManagedThread();
}

void Program::RegisterUnusedFiber(Fiber * aFiber)
{
	// Println(L"Registering fiber % as unused", aFiber->GetName());

	std::unique_lock<std::mutex> lck(GetFiberMutex());
	GetUnusedFibers().Push(aFiber);
}

void Program::QueueFiber(Fiber * aFiber)
{
	std::unique_lock<std::mutex> lck(GetWorkMutex());
	GetQueuedFibers().Add(aFiber);
	lck.unlock();

	GetWorkNotify().notify_one();
}

void Program::QueueBackgroundWork(void(*aFunction)(void*), void* aArgument)
{
	GetBackgroundWorkThreadPool()->QueueSingle([aFunction, aArgument]()
		{
			aFunction(aArgument);
		});
}

Fiber* Program::GetUnusedFiber()
{
	scoped_lock (GetFiberMutex())
	{
		Fiber* fiber;
		if (GetUnusedFibers().TryPop(fiber))
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

Ptr<Object> Program::NewObjectByType(const TypeID<CogTypeBase>& aObject, Object* aParent)
{
	ObjectPool& objectPool = GetObjectPool();
	return objectPool.CreateObjectByType(aObject, aParent);
}

void Program::TickDestroys()
{
	Array<Object*> destroyNow;

	scoped_lock (GetDestroyMutex())
	{

		Array<Array<Object*>>& scheduledDestroys = GetScheduledDestroys();

		if (scheduledDestroys.GetLength() == 0)
			return;

		destroyNow = Move(scheduledDestroys[0]);

		for (i32 i = 1; i < scheduledDestroys.GetLength(); ++i)
		{
			scheduledDestroys[i - 1] = Move(scheduledDestroys[i]);
		}
	}

	for (Object* obj : destroyNow)
		obj->ReturnToAllocator();

	destroyNow.Empty();
	
	scoped_lock (GetDestroyMutex())
		GetScheduledDestroys().Last() = Move(destroyNow);
}

void Program::ScheduleDestruction(Object& aObject)
{
	scoped_lock(GetDestroyMutex())
		GetScheduledDestroys().Last().Add(&aObject);
}
