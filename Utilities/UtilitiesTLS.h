#pragma once

class Fiber;
class ThreadID;
class String;
class StringView;
struct FiberResumeData;

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
}
