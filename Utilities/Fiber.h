#pragma once
#include "FunctionView.h"

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
	
	static void YieldExecution();

private:
	static void ExecuteFiberLoop(void*);
	static thread_local void* ourFiberHandle;

	// Fiber handles
	void* myFiberHandle = nullptr;
	void* myCallingFiber = nullptr;

	void (*myCurrentWork)(void*) = nullptr;
	void* myArgument = nullptr;
};
