#include "pch.h"
#include "Awaitable.h"
#include "Fiber.h"
#include "Program.h"

Awaitable::Awaitable()
{
	CHECK(Program::Get().IsInManagedThread());

	myWaitingFiber = nullptr;
}

void Awaitable::StartWaiting()
{
	if (!IsReady())
	{
		myWaitingFiber = Fiber::GetCurrentlyExecutingFiber();
		Fiber::YieldExecution(this);
		myWaitingFiber = nullptr;
	}
}
