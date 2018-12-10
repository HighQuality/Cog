#pragma once

template <typename T>
class EventListBase
{
public:
	static void Submit(T&& aElement)
	{
		ourLocalElements.Add(Move(aElement));
	}

	static void Submit(const T& aElement)
	{
		Submit(T(aElement));
	}

	static Array<T> Gather()
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

	static thread_local Array<T> ourLocalElements;
	static Array<EventListBase*> ourElementLists;
	static std::mutex ourElementListsMutex;
};
