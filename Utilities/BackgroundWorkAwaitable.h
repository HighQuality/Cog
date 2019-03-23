#pragma once
#include "AwaitableSignal.h"

class BackgroundWorkAwaitable : public AwaitableSignal
{
public:
	BackgroundWorkAwaitable()
	{
		Program::Get().QueueBackgroundWork(&DoBackgroundWork, this);
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
