#include "CorePch.h"
#include "CoreTLS.h"
#include "Threading/ThreadID.h"
#include <External/pcg32.h>
#include <chrono>

static u64 GetRandomSeed()
{
	std::chrono::high_resolution_clock clock;
	return clock.now().time_since_epoch().count();
}

static thread_local ThreadID* ourThreadID = nullptr;
static thread_local String ourThreadName;

static thread_local pcg32 ourRandomEngine(GetRandomSeed());

struct TlsDestruct
{
	NOINLINE ~TlsDestruct()
	{
		ThreadID::DestroyThreadID(ourThreadID);
		ourThreadID = nullptr;
	}
};

static thread_local TlsDestruct ourTlsDestruct;

namespace CoreTLS
{
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

	NOINLINE pcg32& GetRandomEngine()
	{
		return ourRandomEngine;
	}
}
