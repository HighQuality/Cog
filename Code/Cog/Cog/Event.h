#pragma once
#include <Containers/EventList.h>
#include "Pointer.h"

// TODO: Rewrite this entire thing to use ObjectFunctionView

enum class EventCallResult
{
	RemoveSubscription,
	KeepSubscription
};

template <typename ...TArgs>
class Event
{
public:
	Event() = default;

	~Event()
	{
		for (EventSubscriberBase* subscriber : mySubscribers)
			delete subscriber;
	}

	template <typename TType>
	void Subscribe(TType* aObject, void (TType::*aFunction)(TArgs&& ...))
	{
		CHECK(aObject);
		Subscribe(*aObject, aFunction);
	}

	template <typename TType>
	void Subscribe(const TType* aObject, void (TType::*aFunction)(TArgs&& ...) const)
	{
		CHECK(aObject);
		Subscribe(*aObject, aFunction);
	}

	template <typename TType>
	void Subscribe(TType& aObject, void (TType::*aFunction)(TArgs&& ...))
	{
		mySubscribers.Add(new EventSubscriber(Ptr<TType>(&aObject), aFunction));
	}

	template <typename TType>
	void Subscribe(const TType& aObject, void (TType::*aFunction)(TArgs&& ...) const)
	{
		mySubscribers.Add(new EventSubscriber(Ptr<TType>(&const_cast<TType&>(aObject)), aFunction));
	}

	void Broadcast(TArgs ... aArgs)
	{
		const i32 c = mySubscribers.GetLength();

		for (i32 i = c - 1; i >= 0; --i)
		{
			const EventCallResult result = mySubscribers[i]->Call(aArgs...);

			if (result == EventCallResult::RemoveSubscription)
				mySubscribers.RemoveAtSwap(i);
		}
	}

private:
	class EventSubscriberBase
	{
	public:
		virtual ~EventSubscriberBase() = default;

		virtual EventCallResult Call(const TArgs& ... aArgs) = 0;
	};

	template <typename TType, typename TCallback>
	class EventSubscriber final : public EventSubscriberBase
	{
	public:
		Ptr<TType> myObject;
		TCallback myFunction;

		EventSubscriber(Ptr<TType> aObject, TCallback aFunction)
		{
			myObject = Move(aObject);
			myFunction = Move(aFunction);
		}

		EventCallResult Call(const TArgs& ... aArgs) override
		{
			if (!myObject.IsValid())
				return EventCallResult::RemoveSubscription;

			if constexpr (IsSame<decltype(((*myObject).*myFunction)(aArgs...)), EventCallResult>)
				return ((*myObject).*myFunction)(aArgs...);

			((*myObject).*myFunction)(aArgs...);
			return EventCallResult::KeepSubscription;
		}
	};

	Array<EventSubscriberBase*> mySubscribers;
};
