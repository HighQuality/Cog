#include "CogPch.h"
#include "Fiber.h"
#include <Threading/ThreadID.h>
#include "FiberResumeData.h"
#include <Cog/CogTLS.h>

static std::mutex gFiberIndexMutex;
static Array<Fiber*> gFiberIndex;

void Fiber::ExecuteFiberWork(void* aPtr)
{
	Fiber& fiber = *static_cast<Fiber*>(aPtr);

	// Fiber was executed without having any work provided
	CHECK(fiber.myCurrentWork);

	fiber.myCurrentWork(fiber.myArgument);

	FATAL(L"Fiber finished execution (should never happen)");
}

Fiber::Fiber(StringView aName)
	: myName(aName)
{
	// TODO: Remove/disable for performance, only used for debugging
	std::unique_lock<std::mutex> lck(gFiberIndexMutex);
	gFiberIndex.Add(this);
}

Fiber::Fiber(StringView aName, void(*aWork)(void*), void* aArgument)
	: Fiber(aName)
{
	myCurrentWork = aWork;
	myArgument = aArgument;

	const i32 stackSizeKB = 16;
	myFiberHandle = FiberHandle(CreateFiber(stackSizeKB * 1024, &Fiber::ExecuteFiberWork, this));
	CHECK(myFiberHandle);
}

void Fiber::SetWorkDescription(const StringView& aWorkDescription)
{
	myWorkDescription = aWorkDescription;
}

Fiber::~Fiber()
{
	if (myFiberHandle.GetHandle())
	{
		DeleteFiber(myFiberHandle.GetHandle());
		myFiberHandle = FiberHandle();
	}
}

FiberResumeData Fiber::Resume(const FiberResumeData& aResumeData)
{
	CogTLS::SetFiberResumeData(aResumeData);

	if (false)
	{
		Fiber& currentFiber = *Fiber::GetCurrentlyExecutingFiber();

		Println(L"Fiber % (%) switch to fiber % (%) (Thread %)", currentFiber.GetName(), currentFiber.GetWorkDescription(), GetName(), GetWorkDescription(), ThreadID::GetName());
	}

	SwitchToFiber(myFiberHandle.GetHandle());

	return CogTLS::RetrieveFiberResumeData();
}

Fiber* Fiber::ConvertCurrentThreadToFiber(StringView aName)
{
	CHECK(CogTLS::GetThisThreadsStartingFiber() == nullptr);

	Fiber* fiber = new Fiber(aName);
	fiber->myFiberHandle = FiberHandle(ConvertThreadToFiber(fiber));
	CHECK(fiber->myFiberHandle);

	CogTLS::SetThisThreadsStartingFiber(fiber);
	CogTLS::SetProhibitAwaits(false);

	return fiber;
}

Fiber* Fiber::GetCurrentlyExecutingFiber()
{
	return static_cast<Fiber*>(GetFiberData());
}
