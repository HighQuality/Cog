#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include "AwaitableWorkItem.h"

class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();

	template <typename TWork>
	auto QueueSingle(TWork aWork)
	{
		std::unique_lock<std::mutex> lk(myWorkMutex);
		auto result = QueueSingleImpl(Move(aWork));
		lk.unlock();
		myWorkNotify.notify_one();
		return result;
	}

	template <typename... TWork>
	auto QueueMultiple(TWork... aWork)
	{
		std::unique_lock<std::mutex> lk(myWorkMutex);
		auto result = std::make_tuple(QueueSingleImpl(Move(aWork))...);
		lk.unlock();
		// TODO: Look into if this makes any sense
		for (i32 i = 0; i<sizeof...(aWork); ++i)
			myWorkNotify.notify_one();
		return result;
	}

	template <typename TWork>
	auto QueueSingleAwaitable(TWork aWork)
	{
		std::unique_lock<std::mutex> lk(myWorkMutex);
		auto result = QueueSingleAwaitableImpl(Move(aWork));
		lk.unlock();
		myWorkNotify.notify_one();
		return result;
	}

	template <typename... TWork>
	auto QueueMultipleAwaitable(TWork... aWork)
	{
		std::unique_lock<std::mutex> lk(myWorkMutex);
		auto result = std::make_tuple(QueueSingleAwaitableImpl(Move(aWork))...);
		lk.unlock();
		// TODO: Look into if this makes any sense
		for (i32 i=0; i<sizeof...(aWork); ++i)
			myWorkNotify.notify_one();
		return result;
	}

	void Barrier();
	std::shared_ptr<AwaitableBarrierWorkItem> AwaitableBarrier();

	void WaitForCompletion()
	{
		AwaitableBarrier()->Wait();
	}

	static ThreadPool& Get()
	{
		static ThreadPool instance;
		return instance;
	}

private:
	void Worker(i32 aThreadId);

	template <typename TWork>
	std::shared_ptr<AwaitableWorkItem<std::invoke_result_t<TWork>>> QueueSingleAwaitableImpl(TWork aWork)
	{
		auto ptr = std::make_shared<AwaitableLambdaWorkItem<TWork>>(Move(aWork));
		myWorkQueue.Emplace(ptr);
		return ptr;
	}

	template <typename TWork>
	void QueueSingleImpl(TWork aWork)
	{
		myWorkQueue.Emplace(std::make_shared<LambdaWorkItem<TWork>>(Move(aWork)));
	}

	std::mutex myWorkMutex;
	std::condition_variable myWorkNotify;
	Array<std::thread> myWorkers;
	Array<std::shared_ptr<WorkItemBase>> myWorkQueue;
	bool myIsStopping = false;
};
