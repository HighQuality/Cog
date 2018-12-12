#pragma once
#include "EventListBase.h"
#include "ObjectFunctionView.h"

template <typename T>
class Callback;

template <typename ...TArgs>
class Callback<void(TArgs ...)> : EventListBase<BaseFunctionView*>
{
	using Base = EventListBase<BaseFunctionView*>;

public:
	template <typename TType>
	static void Register(TType& aObject, void (TType::*aFunction)(TArgs ...))
	{
		Submit(new ObjectFunctionView(aObject, aFunction));
	}

	static Array<BaseFunctionView*> Gather()
	{
		return Base::Gather();
	}

private:
};
