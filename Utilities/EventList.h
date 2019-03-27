#pragma once
#include "ThreadID.h"

template <typename T>
class EventList
{
public:
	EventList()
	{
		for (auto& listPointer : myNonEmptyArrays)
			listPointer = nullptr;
		myNextEmptyArrayIndex.store(0);
	}

	void Submit(T&& aElement)
	{
		Array<T>& list = FindLocalList();

		if (list.GetLength() == 0)
			myNonEmptyArrays[myNextEmptyArrayIndex.fetch_add(1)] = &list;

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
		i32 count = 0;

		for (Array<T>* list : myNonEmptyArrays)
		{
			if (list == nullptr)
				break;

			count += list->GetLength();
		}

		elements.PrepareAdd(count);

		for (Array<T>* list : myNonEmptyArrays)
		{
			if (list == nullptr)
				break;

			for (T& element : *list)
				elements.Add(Move(element));
			list->Clear();
		}

		for (auto& listPointer : myNonEmptyArrays)
		{
			if (listPointer == nullptr)
				break;

			listPointer = nullptr;
		}

		myNextEmptyArrayIndex.store(0);
	}

private:
	FORCEINLINE Array<T>& FindLocalList()
	{
		return myLocalElements[ThreadID::Get().GetInteger()];
	}

	std::array<Array<T>, 256> myLocalElements;
	std::array<Array<T>*, 256> myNonEmptyArrays;
	std::atomic_int32_t myNextEmptyArrayIndex;
};
