#pragma once
#include <TypeFundamentals/TypeID.h>
#include <Threading/ThreadID.h>
#include <Time/Stopwatch.h>
#include "Threading/Fibers/Await.h"
#include "Pointer.h"
#include "QueuedProgramWork.h"
#include "Program.generated.h"

class Object;
class ThreadPool;
class Fiber;

COGTYPE(SetDebugFlag)
class Program : public Object
{
	GENERATED_BODY;
	
public:
	Program();
	virtual ~Program();

	void Run(bool aPrintDebugInfo);

	static Program& Create();
	static void Destroy();

	static Program& Get();

	bool IsInMainThread() const;
	bool IsInManagedThread() const;

	void RegisterUnusedFiber(Fiber* aFiber);

	template <typename TExtraData>
	void QueueHighPrioWork(void(*aFunction)(TExtraData*), TExtraData* aArgument)
	{
		std::unique_lock<std::mutex> lck(WorkMutex());
		HighPrioWorkQueue().Add({ reinterpret_cast<void(*)(void*)>(aFunction), aArgument });
		lck.unlock();

		WorkNotify().notify_one();
	}

	template <typename TExtraData>
	void QueueWork(void(*aFunction)(TExtraData*), TExtraData* aArgument)
	{
		std::unique_lock<std::mutex> lck(WorkMutex());
		WorkQueue().Add({ reinterpret_cast<void(*)(void*)>(aFunction), aArgument });
		lck.unlock();

		WorkNotify().notify_one();
	}

	void QueueFiber(Fiber* aFiber);

	void QueueBackgroundWork(void(*aFunction)(void*), void* aArgument);

	// Works inside this may not Await, assumes aWorkItems array is valid for the entirety of the frame
	// template <typename T, typename TExtraData>
	// void QueueWorkEvenly(ArrayView<T*> aWorkItems, void(T::*aWork)(const TExtraData&), const TExtraData& aExtraData)
	// {
	// 	struct Work
	// 	{
	// 		void(T::*work)(const TExtraData&);
	// 		ArrayView<T> workItems;
	// 		const TExtraData* extraData;
	// 		i32 from;
	// 		i32 to;
	// 	};
	// 
	// 	Work* work = new Work();
	// 	work->workItems = aWorkItems;
	// 	work->work = aWork;
	// 	work->extraData = &aExtraData;
	// 
	// 	if (aWorkItems.GetLength() <= 0)
	// 		return;
	// 
	// 	i32 numWorkPerWorker = 1;
	// 	i32 extraWorkForLastWorker = 0;
	//	const i32 numWorkers = GetNumWorkers(); 
	//
	// 	if (aWorkItems.GetLength() > numWorkers)
	// 	{
	// 		aWorkItems.GetLength() / numWorkers;
	// 		extraWorkForLastWorker = aWorkItems.GetLength() - numWorkPerWorker;
	// 	}
	// 
	// 	const i32 numWorks = Min(aWorkItems.GetLength(), numWorkers);
	// 
	// 	std::unique_lock<std::mutex> lck(WorkMutex());
	// 
	// 	i32 from = 0;
	// 
	// 	for (i32 i = 0; i < numWorks; ++i)
	// 	{
	// 		const bool isLast = i + 1 == numWorks;
	// 
	// 		i32 to = numWorkPerWorker;
	// 
	// 		if (isLast)
	// 			to += extraWorkForLastWorker;
	// 
	// 		Work* currentWork = isLast ? work : new Work(*work);
	// 		currentWork->from = from;
	// 		currentWork->to = to;
	// 
	// 		QueueWork([](Work* aWork)
	// 		{
	// 			NO_AWAITS;
	// 
	// 			const i32 from = aWork->from;
	// 			const i32 to = aWork->to;
	// 			const ArrayView<T> works = aWork->works;
	// 			void(T::*work)(const TExtraData&) = aWork->work;
	// 			const TExtraData* extraData = aWork->extraData;
	// 
	// 			for (i32 i=from; i<to; ++i)
	// 				(works[i]->*work)(*extraData);
	// 
	// 			delete aWork;
	// 		}, currentWork);
	// 
	// 		from = to + 1;
	// 	}
	// }


	Fiber* GetUnusedFiber();

private:
	void WorkerThread(i32 aThreadIndex);

	void FiberMain();

	COGPROPERTY(UniquePtr<ThreadPool> BackgroundWorkThreadPool, DirectAccess);

	COGPROPERTY(std::mutex FiberMutex, DirectAccess);
	COGPROPERTY(Stack<Fiber*> UnusedFibers, DirectAccess);

	COGPROPERTY(std::mutex WorkMutex, DirectAccess);
	COGPROPERTY(std::condition_variable WorkNotify, DirectAccess);
	COGPROPERTY(Array<std::thread> Workers, DirectAccess);

	COGPROPERTY(bool IsStopping = false);

	COGPROPERTY(std::condition_variable WakeMainNotify, DirectAccess);
	COGPROPERTY(std::mutex WakeMainMutex, DirectAccess);

	COGPROPERTY(i32 SleepingThreads = 0);

	COGPROPERTY(i32 NumWorkers);

	COGPROPERTY(Array<QueuedProgramWork> HighPrioWorkQueue, DirectAccess);
	COGPROPERTY(Array<QueuedProgramWork> WorkQueue, DirectAccess);
	COGPROPERTY(Array<Fiber*> QueuedFibers, DirectAccess);

	COGPROPERTY(bool IsMainRunning = true);
	
	COGPROPERTY(Stopwatch Watch, DirectAccess);
	COGPROPERTY(i32 Frames);
	COGPROPERTY(i32 ElapsedSeconds);

	COGPROPERTY(const ThreadID* MainThreadID);
};

extern Program* gProgram;

FORCEINLINE Program& Program::Get()
{
	return *gProgram;
}
