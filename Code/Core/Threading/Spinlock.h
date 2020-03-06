#pragma once

class Spinlock
{
public:
	FORCEINLINE void Lock()
	{
		while (myFlag.exchange(true, std::memory_order_acquire))
			YieldProcessor();
	}

	FORCEINLINE void Unlock()
	{
		myFlag.store(false, std::memory_order_release);
	}

	// std::mutex interface
	FORCEINLINE void lock() { Lock(); }
	FORCEINLINE void unlock() { Unlock(); }

private:
	std::atomic_bool myFlag = false;
};
