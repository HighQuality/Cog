#pragma once
#include <Threading/ThreadID.h>

template <typename T>
class ThreadLocalList
{
public:
	ThreadLocalList()
	{
		memset(myNonEmptyIndices.data(), 0, sizeof myNonEmptyIndices);
		myNextEmptyArrayIndex.store(0);
	}

	void Submit(T&& aElement)
	{
		u8& index;
		Array<T>& list = FindLocalList(index);

		if (list.GetLength() == 0)
			myNonEmptyArrays[myNextEmptyArrayIndex.fetch_add(1)] = index;

		list.Add(Move(aElement));
	}

	FORCEINLINE void Submit(const T& aElement)
	{
		Submit(T(aElement));
	}

	Array<T> Gather()
	{
		Array<T> elements;
		GatherInto(elements);
		return elements;
	}

	void GatherInto(Array<T>& elements)
	{
		const i32 numberOfLists = myNextEmptyArrayIndex.exchange(0);

		elements.PrepareAdd(numberOfLists);

		for (i32 i = 0; i < numberOfLists; ++i)
		{
			for (T& element : myLocalElements[i])
				elements.Add(Move(element));
			myLocalElements[i].Clear();
		}

		memset(myNonEmptyIndices.data(), 0, sizeof myNonEmptyIndices);
	}

private:
	FORCEINLINE Array<T>& FindLocalList(u8& aOutIndex)
	{
		// Thread IDs start at 1
		aOutIndex = ThreadID::Get().GetInteger() - 1;
		return myLocalElements[aOutIndex];
	}

	std::array<Array<T>, MaxThreadID> myLocalElements;
	std::array<u8, MaxThreadID> myNonEmptyIndices;
	// TODO: Find suitable type through template using MaxThreadID?
	std::atomic_uint8_t myNextEmptyArrayIndex;
};
