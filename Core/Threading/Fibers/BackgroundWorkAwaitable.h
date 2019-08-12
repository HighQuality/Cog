#pragma once
#include "Awaitable.h"
#include "Program.h"

template <typename T>
class BackgroundWorkAwaitable : public Awaitable<T>
{
public:
	using Base = Awaitable;
	using ReturnType = T;

	void TriggerWork() final
	{
		Program::Get().QueueBackgroundWork(&DoBackgroundWork, this);
	}
	
private:
	static void DoBackgroundWork(void* aArg)
	{
		BackgroundWorkAwaitable& work = *static_cast<BackgroundWorkAwaitable*>(aArg);
		work.DoWork();
	}
};
