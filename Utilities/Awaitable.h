#pragma once

class Fiber;

enum class AwaitableType
{
	Polling,
	Pushing
};

class Awaitable
{
public:
	Awaitable();
	Awaitable(AwaitableType aAwaitableType);

	Awaitable(const Awaitable&) = delete;
	Awaitable(Awaitable&&) = delete;

	Awaitable& operator=(const Awaitable&) = delete;
	Awaitable& operator=(Awaitable&&) = delete;

	virtual bool IsReady() const = 0;
	virtual bool StartWaiting();
	virtual ~Awaitable();

	FORCEINLINE bool UsePolling() const { return myAwaitableType == AwaitableType::Polling; }

protected:
	Awaitable(bool bUsePolling);
	Fiber* myWaitingFiber;

private:
	AwaitableType myAwaitableType = AwaitableType::Polling;
};
