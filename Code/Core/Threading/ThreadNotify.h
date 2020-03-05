#pragma once

class ThreadNotify
{
public:
	ThreadNotify();
	ThreadNotify(bool aIsSet);
	~ThreadNotify();

	bool WaitAndReset();
	bool WaitAndReset(const TimeSpan aTimeout);

	bool WaitWithoutReset();
	bool WaitWithoutReset(const TimeSpan aTimeout);

	void Set();
	void Reset();

private:
	bool WaitImplementation(const TimeSpan* aTimeout, bool aShouldReset);

	std::mutex myMutex;
	std::condition_variable myNotify;
	bool myIsSet = false;
};
