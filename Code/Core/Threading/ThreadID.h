#pragma once

using ThreadIDInteger = u8;

constexpr ThreadIDInteger MaxThreadID = 128;

static_assert(MaxThreadID != MaxOf<ThreadIDInteger>, "Max thread ID may not be the max of it's type");

class ThreadID
{
public:
	FORCEINLINE ThreadIDInteger GetInteger() const
	{
		return myID;
	}

	static const ThreadID& Get()
	{
		if (const ThreadID* threadID = CoreTLS::GetThreadID())
			return *threadID;

		ThreadID* newThreadID = new ThreadID();
		CoreTLS::SetThreadID(newThreadID);
		return *newThreadID;
	}

	static StringView GetName()
	{
		return CoreTLS::GetThreadName();
	}

	static void SetName(String aThreadName);

	bool operator==(const ThreadID& aOther) const
	{
		return myID == aOther.myID;
	}

	bool operator!=(const ThreadID& aOther) const
	{
		return myID != aOther.myID;
	}
	
	static void DestroyThreadID(ThreadID* aThreadID)
	{
		delete aThreadID;
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

		for (ThreadIDInteger i = 1; i < MaxThreadID; ++i)
		{
			if (ourThreadIDs[i] == this)
			{
				ourThreadIDs[i] = nullptr;
				return;
			}
		}

		FATAL(L"ThreadID not found on destruction!");
	}

	ThreadIDInteger myID;

	static std::mutex ourMutex;

	// Static variables are zero-initialized
	static std::array<ThreadID*, MaxThreadID> ourThreadIDs;
};
