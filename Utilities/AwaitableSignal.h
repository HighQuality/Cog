#pragma once
#include "Awaitable.h"
#include "Program.h"

enum class AwaitableSignalStatus
{
	Unused,
	NotSignaled,
	Signaled
};

class AwaitableSignal : public Awaitable
{
public:
	AwaitableSignal();

	bool IsReady() const final
	{
		return myIsSignaled;
	}

	static i32 GatherSignaledFibers(Array<Fiber*>& aSignaledFibers);
	static bool IsWaitingOnAnySignals();

	static void SignalIndex(i32 aIndex);
	
	void Signal()
	{
		SignalIndex(mySignalIndex);
	}

	bool StartWaiting() override;

private:
	i32 mySignalIndex;
	bool myIsSignaled;

	static std::mutex ourMutex;
	static AwaitableSignalStatus* ourSignals;
	static Array<AwaitableSignal*> ourAwaitables;
	static i32 ourFirstFreeIndex;
};
