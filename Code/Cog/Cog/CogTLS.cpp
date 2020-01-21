#include "CogPch.h"
#include "CogTLS.h"
#include "Threading/ThreadID.h"
#include "Threading/Fibers/FiberResumeData.h"

static thread_local Fiber* ourThisThreadsStartingFiber = nullptr;
static thread_local FiberResumeData ourFiberResumeData;
static thread_local bool ourProhibitAwaits = true;

namespace UtilitiesTLS
{
	NOINLINE void SetThisThreadsStartingFiber(Fiber* aNewFiber)
	{
		ourThisThreadsStartingFiber = aNewFiber;
	}

	NOINLINE Fiber* GetThisThreadsStartingFiber()
	{
		return ourThisThreadsStartingFiber;
	}

	NOINLINE void SetFiberResumeData(const FiberResumeData& aResumeData)
	{
		ourFiberResumeData = aResumeData;
	}

	NOINLINE FiberResumeData RetrieveFiberResumeData()
	{
		FiberResumeData resumeData = ourFiberResumeData;
		ourFiberResumeData = FiberResumeData();
		return resumeData;
	}

	NOINLINE void SetProhibitAwaits(bool aProhibitAwaits)
	{
		ourProhibitAwaits = aProhibitAwaits;
	}

	NOINLINE bool GetProhibitAwaits()
	{
		return ourProhibitAwaits;
	}
}
