 #pragma once
#include <TypeFundamentals/TypeID.h>
#include <Threading/ThreadID.h>
#include <Time/Stopwatch.h>
#include "Threading/Fibers/Await.h"
#include "Pointer.h"

class Object;
class ThreadPool;
class Fiber;

template <typename T>
T& DefaultAllocate();
template <typename T>
void DefaultFree(T& aObject);

class Program
{
public:
	Program();
	~Program();

	void Run(bool aPrintDebugInfo);

	static Program& Create();
	static void Destroy();

	static Program& Get();

	bool IsInMainThread() const { return myMainThread == ThreadID::Get(); }
	bool IsInManagedThread() const;

	void RegisterUnusedFiber(Fiber* aFiber);

	template <typename TExtraData>
	void QueueHighPrioWork(void(*aFunction)(TExtraData*), TExtraData* aArgument)
	{
		std::unique_lock<std::mutex> lck(myWorkMutex);
		myHighPrioWorkQueue.Add({ reinterpret_cast<void(*)(void*)>(aFunction), aArgument });
		lck.unlock();

		myWorkNotify.notify_one();
	}

	template <typename TExtraData>
	void QueueWork(void(*aFunction)(TExtraData*), TExtraData* aArgument)
	{
		std::unique_lock<std::mutex> lck(myWorkMutex);
		myWorkQueue.Add({ reinterpret_cast<void(*)(void*)>(aFunction), aArgument });
		lck.unlock();

		myWorkNotify.notify_one();
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
	// 
	// 	if (aWorkItems.GetLength() > myNumWorkers)
	// 	{
	// 		aWorkItems.GetLength() / myNumWorkers;
	// 		extraWorkForLastWorker = aWorkItems.GetLength() - numWorkPerWorker;
	// 	}
	// 
	// 	const i32 numWorks = Min(aWorkItems.GetLength(), myNumWorkers);
	// 
	// 	std::unique_lock<std::mutex> lck(myWorkMutex);
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

	const ThreadID& myMainThread;
	ThreadPool* myBackgroundWorkThreadPool;

	std::mutex myFiberMutex;
	Stack<Fiber*> myUnusedFibers;

	std::mutex myWorkMutex;
	std::condition_variable myWorkNotify;
	Array<std::thread> myWorkers;

	bool myIsStopping = false;

	std::condition_variable myWakeMainNotify;
	std::mutex myWakeMainMutex;

	i32 mySleepingThreads = 0;

	i32 myNumWorkers;

	struct QueuedWork
	{
		void(*function)(void*) = nullptr;
		void* argument = nullptr;
	};

	Array<QueuedWork> myHighPrioWorkQueue;
	Array<QueuedWork> myWorkQueue;
	Array<Fiber*> myQueuedFibers;

	bool myIsMainRunning = true;
	
	Stopwatch myWatch;
	i32 myFrames = 0;
	i32 myElapsedSeconds = 0;

};

extern Program* gProgram;

FORCEINLINE Program& Program::Get()
{
	return *gProgram;
}
