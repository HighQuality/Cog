#include "pch.h"
#include "ThreadPool.h"
#include "Semaphore.h"

std::unique_ptr<ThreadPool> ThreadPool::ourThreadPool;

ThreadPool::ThreadPool()
	: ThreadPool(CastBoundsChecked<i32>(std::thread::hardware_concurrency()))
{
}

ThreadPool::ThreadPool(const i32 aNumWorkers)
{
	Println(L"Starting thread pool with % workers...", aNumWorkers);

	for (i32 i = 0; i < aNumWorkers; ++i)
		myWorkers.Emplace(std::thread(&ThreadPool::Worker, this, i));
}

void ThreadPool::Worker(i32 aThreadId)
{
	std::shared_ptr<WorkItemBase> item;

	for (;;)
	{
		for (;;)
		{
			std::unique_lock<std::mutex> lk(myWorkMutex);

			if (myWorkQueue.GetLength() == 0)
			{
				myWorkNotify.wait(lk);
				if (myIsStopping)
					return;
				if (myWorkQueue.GetLength() == 0)
					continue;
			}

			// Barriers are left in the "item" pointer and the work queue, if the barrier we just got rid of is in front of the work queue get rid of it
			if (item == myWorkQueue[0])
			{
				myWorkQueue.RemoveAt(0);
				item.reset();
				continue;
			}

			if (!myWorkQueue[0]->myIsBarrier)
			{
				item = Move(myWorkQueue[0]);
				myWorkQueue.RemoveAt(0);
			}
			else
			{
				item = myWorkQueue[0];
			}

			break;
		}

		item->DoWork();

		if (!item->myIsBarrier)
			item.reset();
	}
}

ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lk(myWorkMutex);
		myIsStopping = true;
	}
	myWorkNotify.notify_all();

	for (i32 i = 0; i < myWorkers.GetLength(); ++i)
		myWorkers[i].join();
}

void ThreadPool::Barrier()
{
	std::unique_lock<std::mutex> lk(myWorkMutex);
	myWorkQueue.Emplace(std::make_shared<BarrierWorkItem>(static_cast<u32>(myWorkers.GetLength())));
	lk.unlock();
	myWorkNotify.notify_all();
}

std::shared_ptr<AwaitableBarrierWorkItem> ThreadPool::AwaitableBarrier()
{
	std::unique_lock<std::mutex> lk(myWorkMutex);
	auto result = std::make_shared<AwaitableBarrierWorkItem>(static_cast<u32>(myWorkers.GetLength()));
	myWorkQueue.Emplace(result);
	lk.unlock();
	myWorkNotify.notify_all();
	return result;
}

void ThreadPool::Pause(Semaphore& aResume, Semaphore& aDoneWithResume)
{
	std::unique_lock<std::mutex> lk(myWorkMutex);

	auto awaitableBarrier = std::make_shared<AwaitableBarrierWorkItem>(static_cast<u32>(myWorkers.GetLength()));
	myWorkQueue.Add(awaitableBarrier);

	// Make one thread wait for the semaphore to be notified
	QueueSingleImpl([&aResume, &aDoneWithResume] { aResume.Wait(); aDoneWithResume.Notify(); });

	// Wait for the semaphore to be notified
	myWorkQueue.Emplace(std::make_shared<BarrierWorkItem>(static_cast<u32>(myWorkers.GetLength())));

	lk.unlock();
	myWorkNotify.notify_all();

	// Wait for current work to finish before returning
	awaitableBarrier->Wait();
}
