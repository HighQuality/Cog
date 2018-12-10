#pragma once
#include <mutex>
#include "Barrier.h"

class WorkItemBase
{
public:
	WorkItemBase() = default;
	virtual ~WorkItemBase() = default;

protected:
	friend class ThreadPool;
	virtual void DoWork() = 0;
	
	bool myIsBarrier = false;
};

template<typename TResult>
class AwaitableWorkItem;

template<>
class AwaitableWorkItem<void> : public WorkItemBase
{
public:
	void Wait();

protected:
	friend class ThreadPool;
	void DoWork() override;

	std::mutex myMutex;
	std::condition_variable myNotify;
	bool myIsDone = false;
};

template <typename TResult>
class AwaitableWorkItem : public AwaitableWorkItem<void>
{
public:
	using Base = AwaitableWorkItem<void>;

	TResult Wait()
	{
		Base::Wait();
		return myResult;
	}

protected:
	TResult myResult;
};

template <typename TLambda>
class AwaitableLambdaWorkItem : public AwaitableWorkItem<std::invoke_result_t<TLambda>>
{
public:
	using ReturnType = std::invoke_result_t<TLambda>;
	using Base = AwaitableWorkItem<ReturnType>;

	AwaitableLambdaWorkItem(TLambda aLambda)
	{
		new (static_cast<void*>(&myLambda)) TLambda(Move(aLambda));
	}

	~AwaitableLambdaWorkItem()
	{
		reinterpret_cast<TLambda&>(myLambda).~TLambda();
	}

protected:
	void DoWork() override
	{
		if constexpr (!IsSame<ReturnType, void>)
			this->myResult = reinterpret_cast<TLambda&>(myLambda)();
		else
			reinterpret_cast<TLambda&>(myLambda)();
		
		// Marks worker as done
		Base::DoWork();
	}
	
private:
	std::aligned_storage<sizeof(TLambda), alignof(TLambda)> myLambda;
};

template <typename TLambda>
class LambdaWorkItem : public WorkItemBase
{
public:
	using Base = WorkItemBase;

	LambdaWorkItem(TLambda aLambda)
	{
		new (static_cast<void*>(&myLambda)) TLambda(Move(aLambda));
	}

	~LambdaWorkItem()
	{
		reinterpret_cast<TLambda&>(myLambda).~TLambda();
	}

protected:
	void DoWork() override
	{
		reinterpret_cast<TLambda&>(myLambda)();
	}

private:
	std::aligned_storage<sizeof(TLambda), alignof(TLambda)> myLambda;
};

class BarrierWorkItem : public WorkItemBase
{
public:
	using Base = WorkItemBase;
	
	BarrierWorkItem(const u32 aNumThreads)
		: myBarrier(aNumThreads)
	{
		myIsBarrier = true;
	}

	void DoWork() override
	{
		myBarrier.Wait();
	}

private:
	Barrier myBarrier;
};

class AwaitableBarrierWorkItem : public AwaitableWorkItem<void>
{
public:
	using Base = AwaitableWorkItem<void>;

	AwaitableBarrierWorkItem(const u32 aNumThreads)
		: myBarrier(aNumThreads)
	{
		myIsBarrier = true;
	}

	void DoWork() override
	{
		if (myBarrier.Wait())
			Base::DoWork();
	}

private:
	Barrier myBarrier;
};
