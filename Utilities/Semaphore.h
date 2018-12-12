#pragma once
#include "Types.h"

class Semaphore
{
public:
	Semaphore()
	{
		myCount = 0;
	}

	void Notify()
	{
		{
			std::unique_lock<std::mutex> lk(myMutex);
			++myCount;
		}
		myNotify.notify_one();
	}

	void Wait()
	{
		std::unique_lock<std::mutex> lk(myMutex);
		while (myCount == 0)
			myNotify.wait(lk);
		--myCount;
	}

	bool TryWait()
	{
		std::unique_lock<std::mutex> lk(myMutex);
		if (myCount == 0)
			return false;
		--myCount;
		return true;
	}

private:
	std::mutex myMutex;
	std::condition_variable myNotify;
	u32 myCount;
};
