#pragma once

class Fiber;

class Awaitable
{
public:
	Awaitable();

	Awaitable(const Awaitable&) = delete;
	Awaitable(Awaitable&&) = delete;

	Awaitable& operator=(const Awaitable&) = delete;
	Awaitable& operator=(Awaitable&&) = delete;

	virtual void StartWork();

	virtual void DoWork() = 0
	{
		SignalWorkFinished();
	}

	virtual ~Awaitable() = default;

protected:
	void SignalWorkFinished();

private:
	static void StartAwaitableWork(void* aAwaitable);

	friend class Await;
	Await* myAwaiter = nullptr;

	bool myWorkFinished = false;
};

#include "Await.h"
