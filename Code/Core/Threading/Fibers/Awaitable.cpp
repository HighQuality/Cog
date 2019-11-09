#include "pch.h"
#include "Awaitable.h"
#include "Fiber.h"
#include "Program.h"

AwaitableBase::AwaitableBase()
{
}

void AwaitableBase::StartAwaitableWork(AwaitableBase* aAwaitable)
{
	aAwaitable->DoWork();
}

void AwaitableBase::TriggerWork()
{
	Program::Get().QueueHighPrioWork(&AwaitableBase::StartAwaitableWork, this);
}

void AwaitableBase::SignalWorkFinished()
{
	CHECK(!myWorkFinished);
	myWorkFinished = true;
	myAwaiter->DecrementCounter();
}
