#pragma once

class Barrier
{
public:
	Barrier(const u32 aNumWaiters)
	{
		myCount = aNumWaiters;
	}

	// Returns true if we were the ones that caused this barrier to be released
	bool Wait()
	{
		std::unique_lock<std::mutex> lk(myMutex);
		
		if (myCount == 0)
			return false;
		
		--myCount;
		
		if (myCount == 0)
		{
			myNotify.notify_all();
			return true;
		}

		while (myCount != 0)
			myNotify.wait(lk);
		return false;
	}

private:
	std::mutex myMutex;
	std::condition_variable myNotify;
	u32 myCount;
};
