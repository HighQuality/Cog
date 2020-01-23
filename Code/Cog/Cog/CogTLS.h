#pragma once

class Fiber;
struct FiberResumeData;

namespace CogTLS
{
	NOINLINE void SetThisThreadsStartingFiber(Fiber* aNewFiber);
	NOINLINE Fiber* GetThisThreadsStartingFiber();

	NOINLINE void SetFiberResumeData(const FiberResumeData& aResumeData);
	NOINLINE FiberResumeData RetrieveFiberResumeData();
	
	NOINLINE void SetProhibitAwaits(bool aProhibitAwaits);
	NOINLINE bool GetProhibitAwaits();

	NOINLINE void MarkThreadAsManaged();
	NOINLINE bool IsInManagedThread();
}
