#pragma once
#include "Awaitable.h"
#include "Program.h"

template <typename T>
class BackgroundWorkAwaitable : public Awaitable
{
public:
	using Base = Awaitable;
	using ReturnType = T;

	void StartWork() final
	{
		Program::Get().QueueBackgroundWork(&DoBackgroundWork, this);
	}
	
	virtual void DoWork() final
	{
		if constexpr (HasData)
			myReturnedData = SynchronousWork();
		else
			SynchronousWork();

		Base::DoWork();
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

		work.SignalWorkFinished();
	}
};
