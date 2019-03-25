#pragma once
#include "AwaitableSignal.h"

template <typename T>
class BackgroundWorkAwaitable : public AwaitableSignal
{
public:
	using ReturnType = T;

	BackgroundWorkAwaitable()
	{
	}
	
	bool StartWaiting() override
	{
		Program::Get().QueueBackgroundWork(&DoBackgroundWork, this);

		return AwaitableSignal::StartWaiting();
	}

	T RetrieveReturnedData()
	{
		if constexpr (HasData)
			return Move(myReturnedData);

		CHECK(false);
	}

protected:
	virtual T SynchronousWork() = 0;

private:
	static constexpr bool HasData = !IsSame<T, void>;
	Select<HasData, T, u8> myReturnedData;

	static void DoBackgroundWork(void* aArg)
	{
		BackgroundWorkAwaitable& work = *static_cast<BackgroundWorkAwaitable*>(aArg);
		
		if constexpr (HasData)
			work.myReturnedData = work.SynchronousWork();
		else
			work.SynchronousWork();

		work.Signal();
	}
};
