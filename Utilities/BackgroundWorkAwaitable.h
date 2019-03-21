#pragma once
#include "AwaitableSignal.h"

#define GENERATE_BACKGROUND_WORK_CONSTRUCTOR_CALL(ThisClass, WorkMethod) \
		: BackgroundWorkAwaitable([](void* aArg) { static_cast<ThisClass*>(aArg)->WorkMethod(); } })

class BackgroundWorkAwaitable : public AwaitableSignal
{
public:
	BackgroundWorkAwaitable()
	{
		Program::Get().QueueBackgroundWork([](void* aArg)
		{
			static_cast<BackgroundWorkAwaitable*>(aArg)->SynchronousWork();
			static_cast<BackgroundWorkAwaitable*>(aArg)->Signal();
		}, this);
	}

protected:
	virtual void SynchronousWork() = 0;
};
