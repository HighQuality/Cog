#pragma once

class Fiber;
class ThreadID;
class String;
class StringView;
struct FiberResumeData;
struct pcg32;

namespace UtilitiesTLS
{
	NOINLINE void SetThisThreadsStartingFiber(Fiber* aNewFiber);
	NOINLINE Fiber* GetThisThreadsStartingFiber();

	NOINLINE void SetThreadID(ThreadID* aNewThreadID);
	NOINLINE const ThreadID* GetThreadID();

	NOINLINE void SetThreadName(String aThreadName);
	NOINLINE StringView GetThreadName();

	NOINLINE void SetFiberResumeData(const FiberResumeData& aResumeData);
	NOINLINE FiberResumeData RetrieveFiberResumeData();
	
	NOINLINE void SetProhibitAwaits(bool aProhibitAwaits);
	NOINLINE bool GetProhibitAwaits();

	NOINLINE pcg32& GetRandomEngine();
}
