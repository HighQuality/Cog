#pragma once
#include "Awaitable.h"
#include "Stopwatch.h"

class AwaitTime : public Awaitable
{
public:
	explicit AwaitTime(Time aAmount)
	{
		myWaitAmount = aAmount;
	}

	bool IsReady() const override
	{
		return myWatch.GetElapsedTime() >= myWaitAmount;
	}

private:
	Stopwatch myWatch;
	Time myWaitAmount;
};
