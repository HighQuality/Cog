#include "pch.h"
#include "AwaitableSignal.h"
#include "Fiber.h"

// Allocate 1 MB
static constexpr i32 gAllocatedSize = 1000000;

static AwaitableSignalStatus* AllocateSignals() { AwaitableSignalStatus* mem = new AwaitableSignalStatus[gAllocatedSize]; memset(mem, CastBoundsChecked<i32>(AwaitableSignalStatus::Unused), gAllocatedSize); return mem; }

std::mutex AwaitableSignal::ourMutex;
// TODO: Fix leak
AwaitableSignalStatus* AwaitableSignal::ourSignals = AllocateSignals();
Array<AwaitableSignal*> AwaitableSignal::ourAwaitables;
i32 AwaitableSignal::ourFirstFreeIndex = 0;

AwaitableSignal::AwaitableSignal()
	: Awaitable(AwaitableType::Pushing)
{
	std::unique_lock<std::mutex> lck(ourMutex);

	mySignalIndex = ourFirstFreeIndex;
	myIsSignaled = false;

	if (ourAwaitables.IsValidIndex(mySignalIndex))
	{
		CHECK(ourAwaitables[mySignalIndex] == nullptr);
		ourAwaitables[mySignalIndex] = this;
	}
	else
	{
		CHECK(mySignalIndex == ourAwaitables.GetLength());
		ourAwaitables.Add(this);
	}

	ourSignals[mySignalIndex] = AwaitableSignalStatus::NotSignaled;

	ourFirstFreeIndex = -1;

	for (i32 i = mySignalIndex + 1; i < gAllocatedSize; ++i)
	{
		if (ourSignals[i] == AwaitableSignalStatus::Unused)
		{
			ourFirstFreeIndex = i;
			break;
		}
	}

	CHECK(ourFirstFreeIndex >= 0);
}

i32 AwaitableSignal::GatherSignaledFibers(Array<Fiber*> & aSignaledFibers)
{
	std::unique_lock<std::mutex> lck(ourMutex);

	ourFirstFreeIndex = -1;

	i32 gatheredSignals = 0;

	// TODO: Optimize

	const i32 n = ourAwaitables.GetLength();
	for (i32 i = 0; i < n; ++i)
	{
		switch (ourSignals[i])
		{
		case AwaitableSignalStatus::Unused:
			if (ourFirstFreeIndex == -1)
				ourFirstFreeIndex = i;
			break;

		case AwaitableSignalStatus::NotSignaled:
			break;

		case AwaitableSignalStatus::Signaled:
			CHECK(ourAwaitables[i]->myWaitingFiber);
			aSignaledFibers.Add(ourAwaitables[i]->myWaitingFiber);
			ourAwaitables[i] = nullptr;
			ourSignals[i] = AwaitableSignalStatus::Unused;

			if (ourFirstFreeIndex == -1)
				ourFirstFreeIndex = i;

			++gatheredSignals;

			break;

		default:
			// Most likely signal of memory corruption if hit
			FATAL(L"Awaitable signal out of range");
		}
	}

	ourAwaitables.ShaveFromEnd(nullptr);

	if (ourFirstFreeIndex == -1)
		ourFirstFreeIndex = ourAwaitables.GetLength();

	return gatheredSignals;
}

bool AwaitableSignal::IsWaitingOnAnySignals()
{
	std::unique_lock<std::mutex> lck(ourMutex);

	return ourAwaitables.GetLength() != 0;
}

void AwaitableSignal::SignalIndex(i32 aIndex)
{
	CHECK(aIndex >= 0);

	// PERF: This lock could potentially be skipped if Program::Get().IsInManagedThread() is true
	std::unique_lock<std::mutex> lck(ourMutex);

	CHECK(aIndex < ourAwaitables.GetLength());
	CHECK(ourSignals[aIndex] != AwaitableSignalStatus::Unused);
	CHECK(ourSignals[aIndex] != AwaitableSignalStatus::Signaled);
	CHECK(!ourAwaitables[aIndex]->myIsSignaled);

	ourSignals[aIndex] = AwaitableSignalStatus::Signaled;
	ourAwaitables[aIndex]->myIsSignaled = true;
}

bool AwaitableSignal::StartWaiting()
{
	const bool bWaited = Awaitable::StartWaiting();

	// If we didn't wait we need to free our slot manually
	if (!bWaited)
	{
		std::unique_lock<std::mutex> lck(ourMutex);
		ourAwaitables[mySignalIndex] = nullptr;
		ourSignals[mySignalIndex] = AwaitableSignalStatus::Unused;
		
		ourAwaitables.ShaveFromEnd(nullptr);
		if (ourFirstFreeIndex > ourAwaitables.GetLength())
			ourFirstFreeIndex = ourAwaitables.GetLength();
	}

	return bWaited;
}
