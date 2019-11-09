#pragma once
#include "ThreadID.h"
#include "CogStack.h"

template <typename T>
class ThreadLocalStack
{
public:
	void Push(T aItem)
	{
		FindOrCreateStack().Push(Move(aItem));
	}

	T Pop()
	{
		return FindOrCreateStack().Pop();
	}

	bool TryPop(T& aItem)
	{
		return FindOrCreateStack().TryPop(aItem);
	}
		
	i32 GetLength() const
	{
		return FindOrCreateStack().GetLength();
	}


private:
	FORCEINLINE Stack<T>& FindOrCreateStack()
	{
		const ThreadID& threadID = ThreadID::Get();
		myLocalStacks.SetMinLength(threadID.GetInteger());
		return myLocalStacks[threadID.GetInteger()];
	}

	Array<Stack<T>> myLocalStacks;
};
