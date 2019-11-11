#include "CorePch.h"
#include "UtilitiesTLS.h"
#include <Threading/ThreadID.h>
#include <Threading/Fibers/FiberResumeData.h>
#include <External/pcg32.h>
#include <chrono>

static u64 GetRandomSeed()
{
	std::chrono::high_resolution_clock clock;
	return clock.now().time_since_epoch().count();
}

static thread_local Fiber* ourThisThreadsStartingFiber = nullptr;
static thread_local ThreadID* ourThreadID = nullptr;
static thread_local String ourThreadName;
static thread_local FiberResumeData ourFiberResumeData;
static thread_local bool ourProhibitAwaits = true;

static thread_local pcg32 ourRandomEngine(GetRandomSeed());

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

	NOINLINE pcg32& GetRandomEngine()
	{
		return ourRandomEngine;
	}
}
