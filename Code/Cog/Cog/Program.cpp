#include "CogPch.h"
#include "Program.h"
#include "Threading/Fibers/Fiber.h"
#include "Threading/Fibers/Awaitable.h"

Program* gProgram = nullptr;

static std::atomic<i32> gNextFiberIndex = 0;

Program::Program()
{
	ThreadID::SetName(String(L"Main Thread"));
	SetMainThreadID(&ThreadID::Get());

	CogTLS::MarkThreadAsManaged();

	BackgroundWorkThreadPool() = MakeUnique<ThreadPool>();

	const auto numWorkers = SetNumWorkers(CastBoundsChecked<i32>(std::thread::hardware_concurrency()));

	Workers().PrepareAdd(numWorkers);

	Println(L"Program starting % worker threads...", numWorkers);

	SetIsMainRunning(false);

	MemoryBarrier();

	for (i32 i = 0; i < numWorkers; ++i)
		Workers().Emplace(std::thread(&Program::WorkerThread, this, i));

	// Wait for all threads to start sleeping
	std::unique_lock<std::mutex> lck(WakeMainMutex());

	while (!IsMainRunning())
		WakeMainNotify().wait(lck);
}

Program::~Program()
{
	Println(L"Program shutting down...");

	BackgroundWorkThreadPool().Clear();

	Println(L"Notifying worker threads to stop...");

	{
		std::unique_lock<std::mutex> lck(WorkMutex());
		SetIsStopping(true);
		// Worker threads don't run while this is true
		SetIsMainRunning(false);
	}

	WorkNotify().notify_all();

	Println(L"Waiting for worker threads to stop...");

	for (std::thread& thread : Workers())
		thread.join();

	SetIsMainRunning(true);

	Println(L"Finished stopping worker threads!");

	Fiber* fiber;
	while (UnusedFibers().TryPop(fiber))
		delete fiber;

	while (QueuedFibers().TryPop(fiber))
		delete fiber;
}

void Program::Run(const bool aPrintDebugInfo)
{
	std::unique_lock<std::mutex> workMutexLck(WorkMutex(), std::defer_lock);

	for (;;)
	{
		workMutexLck.lock();

		const i32 numWork = WorkQueue().GetLength() + HighPrioWorkQueue().GetLength() + QueuedFibers().GetLength();

		if (numWork > 0)
		{
			const i32 numWorkersToWake = Min(numWork, GetNumWorkers());

			if (numWorkersToWake == GetNumWorkers())
			{
				WorkNotify().notify_all();
			}
			else
			{
				// PERF: Try making the switch to notify_all more sensitive
				for (i32 i = 0; i < numWorkersToWake; ++i)
					WorkNotify().notify_one();
			}

			// Wait for all work to finish
			std::unique_lock<std::mutex> wakeMainLck(WakeMainMutex());
			SetIsMainRunning(false);
			workMutexLck.unlock();

			WakeMainNotify().wait(wakeMainLck, [this]() { return IsMainRunning(); });

			workMutexLck.lock();

			// std::this_thread::yield();
		}
		else
		{
			std::unique_lock<std::mutex> fibersLck(FiberMutex());
			// TODO: Make this check "better"?
			// Basically, if the number of unused fibers added to the number of workers equal the next fiber index we have no live fibers waiting on something
			// Each worker thread always have exactly one fiber allocated to itself
			if (UnusedFibers().GetLength() + GetNumWorkers() == gNextFiberIndex)
			{
				if (aPrintDebugInfo)
					Println(L"Program finished in %ms", (static_cast<float>(GetElapsedSeconds()) + GetWatch().GetElapsedTime().Seconds()) * 1000.f);
				break;
			}
		}

		workMutexLck.unlock();

		SetFramesThisSecond(GetFramesThisSecond() + 1);

		if (GetWatch().GetElapsedTime().Seconds() > 1.f)
		{
			Watch().Restart();

			Println(L"% FPS", GetFramesThisSecond());
			SetFramesThisSecond(0);

			SetElapsedSeconds(GetElapsedSeconds() + 1);
		}
	}

	if (aPrintDebugInfo)
		Println(L"Program finished with % fibers allocated", gNextFiberIndex.load());
}

Program& Program::Create()
{
	CHECK(gProgram == nullptr);
	gProgram = new Program();
	return *gProgram;
}

void Program::Destroy()
{
	CHECK(gProgram != nullptr);
	delete gProgram;
	gProgram = nullptr;
}


void Program::FiberMain()
{
	Fiber* currentFiber = Fiber::GetCurrentlyExecutingFiber();

	{
		std::unique_lock<std::mutex> lck(WorkMutex());

		while (!IsStopping())
		{
			if (GetHighPrioWorkQueue().GetLength() > 0)
			{
				QueuedProgramWork work = HighPrioWorkQueue().RemoveAt(0);
				lck.unlock();

				work.function(work.argument);

				lck.lock();
				continue;
			}

			if (GetQueuedFibers().GetLength() > 0)
			{
				Fiber* fiberToExecute = QueuedFibers().Pop();
				lck.unlock();

				FiberResumeData resumeData(FiberResumeType::ResumeFromAwait);
				resumeData.resumeFromAwaitData.sleepingFiber = currentFiber;
				fiberToExecute->Resume(resumeData);

				lck.lock();
				continue;
			}

			if (GetWorkQueue().GetLength() > 0)
			{
				QueuedProgramWork work = WorkQueue().RemoveAt(0);
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
					std::unique_lock<std::mutex> mainLock(WakeMainMutex());
					SetIsMainRunning(true);
				}

				WakeMainNotify().notify_one();
			}

			do
			{
				WorkNotify().wait(lck);

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

	std::unique_lock<std::mutex> lck(FiberMutex());
	UnusedFibers().Push(aFiber);
}

void Program::QueueFiber(Fiber * aFiber)
{
	std::unique_lock<std::mutex> lck(WorkMutex());
	QueuedFibers().Add(aFiber);
	lck.unlock();

	WorkNotify().notify_one();
}

void Program::QueueBackgroundWork(void(*aFunction)(void*), void* aArgument)
{
	BackgroundWorkThreadPool()->QueueSingle([aFunction, aArgument]()
		{
			aFunction(aArgument);
		});
}

Fiber* Program::GetUnusedFiber()
{
	{
		std::unique_lock<std::mutex> lck(FiberMutex());

		Fiber* fiber;
		if (UnusedFibers().TryPop(fiber))
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
