#pragma once

using ThreadIDInteger = u8;

constexpr ThreadIDInteger MaxThreadID = 128;
	
static_assert(MaxThreadID != MaxOf<ThreadIDInteger>, "Max thread ID may not be the max of it's type");

class ThreadID
{
public:
	ThreadIDInteger GetInteger() const
	{
		return myID;
	}

	static const ThreadID& Get()
	{
		return ourThreadID;
	}

	bool operator==(const ThreadID& aOther) const
	{
		return myID == aOther.myID;
	}

	bool operator!=(const ThreadID& aOther) const
	{
		return myID != aOther.myID;
	}

private:
	ThreadID()
	{
		std::unique_lock<std::mutex> lck(ourMutex);

		for (ThreadIDInteger i = 1; i < MaxThreadID; ++i)
		{
			if (ourThreadIDs[i] == nullptr)
			{
				ourThreadIDs[i] = this;
				myID = i;
				return;
			}
		}

		FATAL(L"Too many threads running (maximum of {0})!", MaxThreadID);
	}

	~ThreadID()
	{
		std::unique_lock<std::mutex> lck(ourMutex);

		ThreadIDInteger index = MaxOf<ThreadIDInteger>;

		for (ThreadIDInteger i = 1; i < MaxThreadID; ++i)
		{
			if (ourThreadIDs[i] == this)
			{
				index = i;
				break;
			}
		}

		CHECK(index >= 0);
		ourThreadIDs[index] = nullptr;
	}

	ThreadIDInteger myID;

	static thread_local ThreadID ourThreadID;
	static std::mutex ourMutex;

	// Static variables are zero-initialized
	static ThreadID* ourThreadIDs[MaxThreadID];
};
