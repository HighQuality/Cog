#pragma once
#include "AwaitableSignal.h"

class BackgroundWorkAwaitable : public AwaitableSignal
{
public:
	BackgroundWorkAwaitable()
	{
	}
	
	bool StartWaiting() override
	{
		Program::Get().QueueBackgroundWork(&DoBackgroundWork, this);

		return AwaitableSignal::StartWaiting();
	}

protected:
	virtual void SynchronousWork() = 0;

private:
	static void DoBackgroundWork(void* aArg)
	{
		BackgroundWorkAwaitable& work = *static_cast<BackgroundWorkAwaitable*>(aArg);
		work.SynchronousWork();
		work.Signal();
	}
};
