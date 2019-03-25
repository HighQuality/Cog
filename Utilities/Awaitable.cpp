#include "pch.h"
#include "Awaitable.h"
#include "Fiber.h"
#include "Program.h"

Awaitable::Awaitable()
{
}

void Awaitable::StartAwaitableWork(void* aAwaitable)
{
	Awaitable& This = *reinterpret_cast<Awaitable*>(aAwaitable);
	This.DoWork();
	This.SignalWorkFinished();
}

void Awaitable::StartWork()
{
	Program::Get().QueueWork(&Awaitable::StartAwaitableWork, this);
}

void Awaitable::SignalWorkFinished()
{
	myAwaiter->DecrementCounter();
}
