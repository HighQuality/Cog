#pragma once
#include <ThreadID.h>

template <typename T>
class EventListBase
{
public:
	void Submit(T&& aElement)
	{
		FindLocalList().Add(Move(aElement));
	}

	void Submit(const T& aElement)
	{
		Submit(T(aElement));
	}

	Array<T> Gather()
	{
		std::unique_lock<std::mutex> lck(ourElementListsMutex);

		Array<T> elements;

		i32 count = 0;

		for (Array<T>* list : ourElementLists)
			count += list->GetLength();

		elements.PrepareAdd(count);

		for (Array<T>* list : ourElementLists)
		{
			for (T& drawCall : *list)
				elements.Add(Move(drawCall));
			list->Clear();
		}

		return elements;
	}

private:
	EventListBase()
	{
		std::unique_lock<std::mutex> lck(ourElementListsMutex);
		ourElementLists.Add(this);
	}

	~EventListBase()
	{
		std::unique_lock<std::mutex> lck(ourElementListsMutex);
		ourElementLists.RemoveSwap(this);
	}

	Array<T>& FindLocalList()
	{
		return ourLocalElements[ThreadID::Get().GetInteger()];
	}

	std::array<Array<T>, 256> ourLocalElements;
	Array<EventListBase*> ourElementLists;
	std::mutex ourElementListsMutex;
};
