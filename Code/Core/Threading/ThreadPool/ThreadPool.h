﻿#pragma once
#include "AwaitableWorkItem.h"

class Semaphore;

class ThreadPool
{
public:
	ThreadPool();
	explicit ThreadPool(i32 aNumWorkers);

	~ThreadPool();

	template <typename TWork>
	void QueueSingle(TWork aWork)
	{
		std::unique_lock<std::mutex> lk(myWorkMutex);
		QueueSingleImpl(Move(aWork));
		lk.unlock();
		myWorkNotify.notify_one();
	}

	template <typename... TWork>
	void QueueMultiple(TWork... aWork)
	{
		std::unique_lock<std::mutex> lk(myWorkMutex);
		std::make_tuple((QueueSingleImpl(Move(aWork)), 1.f)...);
		lk.unlock();
		// TODO: Look into if this makes any sense
		for (i32 i = 0; i<sizeof...(aWork); ++i)
			myWorkNotify.notify_one();
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

	// Wait for all work to finish and then pause all threads until the given semaphore is notified
	void Pause(Semaphore& aResume, Semaphore& aDoneWithResume);

	static void Create()
	{
		ourThreadPool = std::make_unique<ThreadPool>();
	}

	static void Destroy()
	{
		ourThreadPool.reset();
	}

	static ThreadPool& Get()
	{
		return *ourThreadPool;
	}

private:
	static std::unique_ptr<ThreadPool> ourThreadPool;
	
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
