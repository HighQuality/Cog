#pragma once
#include "FunctionView.h"
#include "FiberHandle.h"
#include "CogStack.h"

class Fiber
{
public:
	Fiber();
	~Fiber();

	DELETE_MOVES(Fiber);

	bool Execute(void (*aWork)(void*), void* aArgument);
	
	void StartWork(void (*aWork)(void*), void* aArgument);
	bool Continue();

	bool HasWork() const { return myCurrentWork != nullptr; }
	
	static void YieldExecution(void* yieldData);

	static void ConvertCurrentThreadToFiber(const StringView& aDescription);

	void* GetYieldedData() const
	{
		CHECK(HasWork());
		return myYieldedData;
	}
	
	void* RetrieveYieldedData()
	{
		CHECK(HasWork());
		void* data = myYieldedData;
		myYieldedData = nullptr;
		return data;
	}

	static Fiber* GetCurrentlyExecutingFiber();

	static void SetCurrentWork(const StringView& aWork);
	void SetWork(const StringView& aWork);

	const FiberHandle& GetFiberHandle() const { return myFiberHandle; }

private:
	static void ExecuteFiberLoop(void*);
	static thread_local void* ourFiberHandle;
	
	FiberHandle myFiberHandle;
	FiberHandle myCallingFiber;

	void (*myCurrentWork)(void*) = nullptr;
	void* myArgument = nullptr;

	void* myYieldedData = nullptr;
};
