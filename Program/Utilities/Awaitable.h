#pragma once

class Fiber;

class AwaitableBase
{
public:
	AwaitableBase();

	AwaitableBase(const AwaitableBase&) = delete;
	AwaitableBase(AwaitableBase&&) = delete;

	AwaitableBase& operator=(const AwaitableBase&) = delete;
	AwaitableBase& operator=(AwaitableBase&&) = delete;

	virtual void TriggerWork();

	virtual void DoWork() = 0
	{
		SignalWorkFinished();
	}
	
	virtual ~AwaitableBase() = default;

protected:
	void SignalWorkFinished();

	virtual void RetrieveReturnedValue(void* aObject, i32 aSize)
	{
	}

private:
	static void StartAwaitableWork(AwaitableBase* aAwaitable);
	
	friend class AwaitContext;
	AwaitContext* myAwaiter = nullptr;

	bool myWorkFinished = false;
};


template <typename TReturn>
class Awaitable : public AwaitableBase
{
public:
	using Base = AwaitableBase;
	using ReturnType = TReturn;

	void DoWork() override
	{
		if constexpr (HasData)
			myReturnedData = Work();
		else
			Work();

		Base::DoWork();	
	}

protected:
	virtual TReturn Work() = 0;

	void RetrieveReturnedValue(void* aObject, i32 aSize) final
	{
		CHECK(aSize == sizeof(TReturn));

		if constexpr (HasData)
			*static_cast<TReturn*>(aObject) = Move(myReturnedData);
	}

private:
	static constexpr bool HasData = !IsSame<TReturn, void>;
	Select<HasData, TReturn, u8> myReturnedData;
};


#include "Await.h"
