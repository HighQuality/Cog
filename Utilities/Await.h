#pragma once
#include "Fiber.h"
#include "Awaitable.h"
 
class AwaitContext
{
public:
	DELETE_MOVES(AwaitContext);

	AwaitContext()
	{
	}

	template <typename T, typename ...TArgs>
	void While(TArgs ...aArgs)
	{
		CHECK(!myWasExecuted);

		if (!myCounter)
			myCounter = new std::atomic<u8>(0);

		AwaitableBase* awaitable = new T(std::forward<TArgs>(aArgs)...);
		awaitable->myAwaiter = this;
		myAwaitables.Add(awaitable);
	}

	~AwaitContext()
	{
		if (myWasExecuted)
		{
			// Decremented to zero and then executed
			CHECK(myCounter->load() == static_cast<u8>(-1));
		}

		delete myFollowedBy;
		myFollowedBy = nullptr;

		delete myCounter;
		myCounter = nullptr;

		for (AwaitableBase* awaitable : myAwaitables)
			delete awaitable;
		myAwaitables.Clear();
	}

	void Execute();

	AwaitContext & Then()
	{
		CHECK(myFollowedBy == nullptr);
		myFollowedBy = new AwaitContext();
		return *myFollowedBy;
	}

	template <typename T, typename ...TArgs>
	AwaitContext& Then(TArgs & ...aArgs)
	{
		CHECK(myFollowedBy == nullptr);
		myFollowedBy = new T(std::forward<TArgs>(aArgs)...);
		return *myFollowedBy;
	}

	void DecrementCounter();

	void RetrieveReturnedValue(void* aObject, i32 aSize)
	{
		myAwaitables[0]->RetrieveReturnedValue(aObject, aSize);
	}

private:
	Array<AwaitableBase*> myAwaitables;
	std::atomic<u8>* myCounter = nullptr;
	AwaitContext* myFollowedBy = nullptr;
	Fiber* mySleepingFiber = nullptr;

	bool myWasExecuted = false;
};

class AwaitExecuterBase
{
protected:
	virtual void RunGeneric() = 0;
	virtual ~AwaitExecuterBase() = default;
};

template <typename TReturn>
class AwaitExecuter : public AwaitExecuterBase
{
public:
	AwaitExecuter() = default;

	AwaitExecuter(AwaitExecuter&& aOther)
	{
		CHECK(!myHasExecuted && !aOther.myHasExecuted);
		Swap(myContext, aOther.myContext);
	}

	AwaitExecuter& operator=(AwaitExecuter&& aOther)
	{
		CHECK(!myHasExecuted);
		Swap(myContext, aOther.myContext);
		return *this;
	}

	~AwaitExecuter()
	{
		if (myContext)
		{
			if (!myHasExecuted)
				Run();

			delete myContext;
			myContext = nullptr;
		}
	}

	template <typename TAwaitableType, typename ...TArgs>
	AwaitContext& Simultaneously(TArgs ...aArgs)
	{
		if (!myContext)
			myContext = new AwaitContext();

		myContext->While<TAwaitableType>(std::forward<TArgs>(aArgs)...);
		return *myContext;
	}
	
	TReturn Run()
	{
		RunGeneric();

		if constexpr (!IsSame<TReturn, void>)
		{
			TReturn returnValue;
			RetrieveReturnedValue(returnValue);
			return returnValue;
		}

		// We don't need an else here since the destructor will call execute
	}

	operator TReturn()
	{
		return Run();
	}
	
protected:
	void RunGeneric() final
	{
		CHECK(myHasExecuted == false);

		myHasExecuted = true;
		
		if (myContext)
			myContext->Execute();
	}
	
private:
	void RetrieveReturnedValue(TReturn& aObject)
	{
		myContext->RetrieveReturnedValue(&aObject, sizeof aObject);
	}

	AwaitContext* myContext = nullptr;
	AwaitExecuterBase* myParent = nullptr;
	bool myHasExecuted = false;
	bool myHasChild = false;
};

template <typename T, typename ...TArgs>
AwaitExecuter<typename T::ReturnType> Await(TArgs & ...aArgs)
{
	// We're either in a fiber where you can't temporarily can't await (look further into the callstack) or in a non-fiber thread
	CHECK(!UtilitiesTLS::GetProhibitAwaits());

	AwaitExecuter<T::ReturnType> awaitExec;
	awaitExec.Simultaneously<T>(std::forward<TArgs>(aArgs)...);
	return awaitExec;
}

class ProhibitAwaits
{
public:
	ProhibitAwaits()
	{
		gPreviousProhibitAwaits = UtilitiesTLS::GetProhibitAwaits();
		UtilitiesTLS::SetProhibitAwaits(true);
	}

	~ProhibitAwaits()
	{
		UtilitiesTLS::SetProhibitAwaits(gPreviousProhibitAwaits);
	}

	DELETE_COPYCONSTRUCTORS_AND_MOVES(ProhibitAwaits);

private:
	bool gPreviousProhibitAwaits;
};

#define NO_AWAITS ProhibitAwaits disableAwaits;
