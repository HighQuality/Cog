#include "pch.h"
#include "ThreadPool.h"

std::unique_ptr<ThreadPool> ThreadPool::ourThreadPool;

ThreadPool::ThreadPool()
{
	const i32 numThreads = std::thread::hardware_concurrency();

	for (i32 i = 0; i < numThreads; ++i)
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
