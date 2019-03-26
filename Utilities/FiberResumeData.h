#pragma once

enum class FiberResumeType
{
	Starting,
	Await,
	ResumeFromAwait,
	Exiting,
	Count
};

struct FiberResumeData
{
	union
	{
		struct 
		{
			Array<class Awaitable*>* workItems;
		} awaitData;

		struct 
		{
			class Fiber* sleepingFiber;
		} resumeFromAwaitData;
	};

	FiberResumeType type;

	FiberResumeData()
	{
		memset(this, 0, sizeof *this);
		type = FiberResumeType::Count;
	}

	explicit FiberResumeData(FiberResumeType aType)
		: FiberResumeData()
	{
		type = aType;
	}
};
