#include "CorePch.h"
#include "Await.h"
#include <Program.h>

void AwaitContext::Execute()
{
	myWasExecuted = true;

	// No work
	if (myAwaitables.GetLength() == 0)
		return;

	myCounter->store(CastBoundsChecked<u8>(myAwaitables.GetLength()));

	mySleepingFiber = Fiber::GetCurrentlyExecutingFiber();

	for (AwaitableBase* awaitable : myAwaitables)
		awaitable->myAwaiter = this;

	// Put this fiber to sleep until we're woken up
	FiberResumeData resumeData;
	resumeData.type = FiberResumeType::Await;
	resumeData.awaitData.workItems = &myAwaitables;
	FiberResumeData returnedData = UtilitiesTLS::GetThisThreadsStartingFiber()->Resume(resumeData);

	CHECK(returnedData.type == FiberResumeType::ResumeFromAwait);
	CHECK(returnedData.resumeFromAwaitData.sleepingFiber);

	Program::Get().RegisterUnusedFiber(returnedData.resumeFromAwaitData.sleepingFiber);

	// Checks that the counter is 0 and prevent other threads from simultaneously succeeding this check
	CHECK(myCounter->fetch_sub(1) == 0);

	mySleepingFiber = nullptr;

	if (myFollowedBy)
		myFollowedBy->Execute();
}

void AwaitContext::DecrementCounter()
{
	const i32 oldValue = myCounter->fetch_sub(1);
	if (oldValue == 1)
	{
		Program::Get().QueueFiber(mySleepingFiber);
	}
	else if (oldValue < 1)
	{
		FATAL(L"Too many awaitable decrements");
	}
}
