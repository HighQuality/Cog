#pragma once
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
class AwaitableLambdaWorkItem : public AwaitableWorkItem<decltype(TLambda::operator())>, TLambda
{
public:
	using ReturnType = decltype(TLambda::operator());
	using Base = AwaitableWorkItem<ReturnType>;

	AwaitableLambdaWorkItem(TLambda aLambda)
		: TLambda(Move(aLambda))
	{
	}

protected:
	void DoWork() override
	{
		if constexpr (!IsSame<ReturnType, void>)
			this->myResult = TLambda::operator()();
		else
			TLambda::operator()();

		// Marks worker as done
		Base::DoWork();
	}
};

template <typename TLambda>
class LambdaWorkItem : public WorkItemBase, TLambda
{
public:
	using Base = WorkItemBase;

	LambdaWorkItem(TLambda aLambda)
		: TLambda(Move(aLambda))
	{
	}

protected:
	void DoWork() override
	{
		TLambda::operator()();
	}
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
