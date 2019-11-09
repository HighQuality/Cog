#include "CorePch.h"
#include "AwaitableWorkItem.h"

void AwaitableWorkItem<void>::Wait()
{
	std::unique_lock<std::mutex> lk(myMutex);
	if (myIsDone)
		return;
	myNotify.wait(lk, [this] { return myIsDone; });
}

void AwaitableWorkItem<void>::DoWork()
{
	std::unique_lock<std::mutex> lk(myMutex);
	myIsDone = true;
	lk.unlock();
	myNotify.notify_all();
}
