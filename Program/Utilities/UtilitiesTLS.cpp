#include "pch.h"
#include "UtilitiesTLS.h"
#include "ThreadID.h"
#include "FiberResumeData.h"

static thread_local Fiber* ourThisThreadsStartingFiber = nullptr;
static thread_local ThreadID* ourThreadID = nullptr;
static thread_local String ourThreadName;
static thread_local FiberResumeData ourFiberResumeData;
static thread_local bool ourProhibitAwaits = true;

struct TlsDestruct
{
	NOINLINE ~TlsDestruct()
	{
		ourThreadID->NotifyThreadShuttingDown();
	}
};

static thread_local TlsDestruct ourTlsDestruct;

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

	NOINLINE void SetThreadID(ThreadID* aNewThreadID)
	{
		ourThreadID = aNewThreadID;
	}

	NOINLINE const ThreadID* GetThreadID()
	{
		return ourThreadID;
	}

	NOINLINE void SetThreadName(String aThreadName)
	{
		ourThreadName = Move(aThreadName);
	}

	NOINLINE StringView GetThreadName()
	{
		return ourThreadName;
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
