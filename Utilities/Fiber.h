#pragma once
#include "FiberHandle.h"
#include "FiberResumeData.h"

class Fiber
{
public:
	Fiber(StringView aName, void (*aWork)(void*), void* aArgument);
	~Fiber();

	DELETE_MOVES(Fiber);

	FiberResumeData Resume(const FiberResumeData& aResumeData);
	
	static Fiber* ConvertCurrentThreadToFiber(StringView aName);

	static Fiber* GetCurrentlyExecutingFiber();

	void SetWorkDescription(const StringView& aWorkDescription);

	const FiberHandle& GetFiberHandle() const { return myFiberHandle; }

	const String& GetName() const { return myName; }
	const String& GetWorkDescription() const { return myWorkDescription; }

private:
	Fiber(StringView aName);
	static void ExecuteFiberWork(void*);
	
	String myName;
	String myWorkDescription;

	FiberHandle myFiberHandle;

	void (*myCurrentWork)(void*) = nullptr;
	void* myArgument = nullptr;
};
