#pragma once

class ThreadID
{
public:
	u8 GetInteger() const
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
		if (ourThreadIDs.GetLength() == 0)
			ourThreadIDs.Resize(255);

		for (i32 i = 1; i < ourThreadIDs.GetLength(); ++i)
		{
			if (ourThreadIDs[i] == nullptr)
			{
				ourThreadIDs[i] = this;
				myID = i;
				return;
			}
		}

		FATAL(L"Too many threads!");
	}

	~ThreadID()
	{
		std::unique_lock<std::mutex> lck(ourMutex);

		const i32 index = ourThreadIDs.Find(this);
		CHECK(index >= 0);
		ourThreadIDs[index] = nullptr;
	}

	u8 myID;

	static thread_local ThreadID ourThreadID;
	static std::mutex ourMutex;
	static Array<ThreadID*> ourThreadIDs;
};
