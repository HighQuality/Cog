#pragma once
#include "Pointer.h"

// TODO: Rewrite this entire thing to use ObjectFunctionView

enum class DelegateCallResult
{
	RemoveSubscription,
	KeepSubscription
};

template <typename ...TArgs>
class Delegate
{
public:
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
		mySubscribers.Add(UniquePtr<DelegateSubscriberBase>(new DelegateSubscriber<TType, decltype(aFunction)>(Ptr<TType>(&aObject), aFunction)));
	}

	template <typename TType>
	void Subscribe(const TType& aObject, void (TType::*aFunction)(TArgs&& ...) const)
	{
		mySubscribers.Add(UniquePtr<DelegateSubscriberBase>(new DelegateSubscriber<TType, decltype(aFunction)>(Ptr<TType>(&const_cast<TType&>(aObject)), aFunction)));
	}

	void Broadcast(TArgs ... aArgs)
	{
		const i32 c = mySubscribers.GetLength();

		for (i32 i = c - 1; i >= 0; --i)
		{
			const DelegateCallResult result = mySubscribers[i]->Call(aArgs...);

			if (result == DelegateCallResult::RemoveSubscription)
				mySubscribers.RemoveAtSwap(i);
		}
	}

private:
	class DelegateSubscriberBase
	{
	public:
		virtual ~DelegateSubscriberBase() = default;

		virtual DelegateCallResult Call(const TArgs& ... aArgs) = 0;
	};

	template <typename TType, typename TCallback>
	class DelegateSubscriber final : public DelegateSubscriberBase
	{
	public:
		Ptr<TType> myObject;
		TCallback myFunction;

		DelegateSubscriber(Ptr<TType> aObject, TCallback aFunction)
		{
			myObject = Move(aObject);
			myFunction = Move(aFunction);
		}

		DelegateCallResult Call(const TArgs& ... aArgs) override
		{
			if (!myObject.IsValid())
				return DelegateCallResult::RemoveSubscription;

			if constexpr (IsSame<decltype(((*myObject).*myFunction)(aArgs...)), DelegateCallResult>)
				return ((*myObject).*myFunction)(aArgs...);

			((*myObject).*myFunction)(aArgs...);
			return DelegateCallResult::KeepSubscription;
		}
	};

	Array<UniquePtr<DelegateSubscriberBase>> mySubscribers;
};
