#include "CorePch.h"
#include "ThreadNotify.h"

ThreadNotify::ThreadNotify()
{
}

ThreadNotify::ThreadNotify(const bool aIsSet)
{
	myIsSet = aIsSet;
}

ThreadNotify::~ThreadNotify()
{
}

bool ThreadNotify::WaitAndReset()
{
	return WaitImplementation(nullptr, true);
}

bool ThreadNotify::WaitAndReset(const TimeSpan aTimeout)
{
	return WaitImplementation(&aTimeout, true);
}

bool ThreadNotify::WaitWithoutReset()
{
	return WaitImplementation(nullptr, false);
}

bool ThreadNotify::WaitWithoutReset(const TimeSpan aTimeout)
{
	return WaitImplementation(&aTimeout, false);
}

bool ThreadNotify::WaitImplementation(const TimeSpan* aTimeout, const bool aShouldReset)
{
	std::unique_lock<std::mutex> lk(myMutex);

	if (myIsSet)
		return true;

	const auto waitFor = [this] { return myIsSet; };

	if (aTimeout)
		myNotify.wait_for(lk, std::chrono::duration<float, std::micro>(aTimeout->GetTotalMicroseconds()), waitFor);
	else
		myNotify.wait(lk, waitFor);

	const bool wasSet = myIsSet;
	
	if (aShouldReset)
		myIsSet = false;
	
	return wasSet;
}

void ThreadNotify::Set()
{
	scoped_lock(myMutex)
	{
		if (myIsSet)
			return;

		myIsSet = true;
		myNotify.notify_one();
	}
}

void ThreadNotify::Reset()
{
	scoped_lock(myMutex)
	{
		myIsSet = false;
	}
}
