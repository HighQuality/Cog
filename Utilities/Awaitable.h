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

	virtual bool IsReady() const = 0;
	virtual bool StartWaiting();
	virtual ~Awaitable();

protected:
	Fiber* myWaitingFiber;
};
