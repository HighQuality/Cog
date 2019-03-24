#include "pch.h"
#include "Awaitable.h"
#include "Fiber.h"
#include "Program.h"

Awaitable::Awaitable()
{
	CHECK(Program::Get().IsInManagedThread());

	myWaitingFiber = nullptr;
}

Awaitable::Awaitable(const AwaitableType aAwaitableType)
	: Awaitable()
{
	myAwaitableType = aAwaitableType;
}

bool Awaitable::StartWaiting()
{
	if (!IsReady())
	{
		myWaitingFiber = Fiber::GetCurrentlyExecutingFiber();
		Fiber::YieldExecution(this);
		myWaitingFiber = nullptr;
		return true;
	}

	return false;
}

Awaitable::~Awaitable()
{
	myWaitingFiber = (Fiber*)2;
}
