#pragma once
#include "Pointer.h"
#include <InlineVirtualObject.h>

template <typename TFunc>
class ObjectFunctionView;

template <typename TReturn, typename ...TArgs>
class ObjectFunctionView<TReturn(TArgs...)>;

template <typename TReturn, typename ...TArgs>
class BaseObjectFunctionViewHelper
{
public:
	virtual ~BaseObjectFunctionViewHelper() = default;
	virtual TReturn Call(TArgs...) const = 0;
	virtual bool IsValid() const = 0;
	virtual ObjectFunctionView<void()> CreateFunction();
};

template <typename TObject, typename TReturn, typename ...TArgs>
class ObjectFunctionViewHelper final : public BaseObjectFunctionViewHelper<TReturn, TArgs...>
{
public:
	ObjectFunctionViewHelper(TObject& aObject, TReturn(TObject::*aMemberFunction)(TArgs...))
	{
		myObjectPointer = aObject;
		myMemberFunction = aMemberFunction;
	}

	TReturn Call(TArgs... aArgs) const final
	{
		(myObjectPointer->*myMemberFunction)(std::forward<TArgs>(aArgs)...);
	}

	bool IsValid() const final
	{
		return myObjectPointer.IsValid() && myMemberFunction;
	}

private:
	Ptr<TObject> myObjectPointer;
	TReturn (TObject::*myMemberFunction)(TArgs...) = nullptr;
};

template <typename TReturn, typename ...TArgs>
class ObjectFunctionView<TReturn(TArgs ...)>
{
public:
	ObjectFunctionView() = default;

	template <typename TObject>
	ObjectFunctionView(TObject& aObject, TReturn(TObject::*aFunction)(TArgs...))
	{
		Initialize<TObject, decltype(aFunction)>(aObject, aFunction);
	}

	template <typename TObject>
	ObjectFunctionView(const TObject& aObject, TReturn(TObject::*aFunction)(TArgs...) const)
	{
		Initialize<const TObject, decltype(aFunction)>(aObject, aFunction);
	}

	ObjectFunctionView(const ObjectFunctionView& aCopy)
		: ObjectFunctionView()
	{
		if (aCopy.IsValid())
		{
			myFunction = 
		}
	}

	explicit operator bool() const { return IsValid(); }

	bool IsValid() const
	{
		return myFunction && myFunction->IsValid();
	}

	template <typename TCallRet = TReturn>
	EnableIf<!IsSame<TCallRet, void>, bool> TryCall(TArgs ...aArgs, TCallRet& aReturnValue) const
	{
		if (!IsValid())
			return false;
		aReturnValue = myFunction->Call(std::forward<TArgs>(aArgs)...);
		return true;
	}

	template <typename TCallRet = TReturn>
	EnableIf<IsSame<TCallRet, void>, bool> TryCall(TArgs ...aArgs) const
	{
		if (!IsValid())
			return false;
		myFunction->Call();
		return true;
	}

	TReturn Call(TArgs ...aArgs) const
	{
		CHECK(IsValid());
		return myFunction->Call(std::forward<TArgs>(aArgs)...);
	}

private:
	template <typename TObject, typename TMemberFunction>
	void Initialize(TObject& aObject, TMemberFunction aMemberFunction)
	{
		myFunction.Store<ObjectFunctionViewHelper<TObject, TReturn, TArgs...>>(aObject, aMemberFunction);
	}

	InlineVirtualObject<BaseObjectFunctionViewHelper<TReturn, TArgs...>, sizeof ObjectFunctionViewHelper<Object, TReturn, TArgs...>> myFunction;
};

// template <typename TType, typename TFunc>
// void MakeObjectFunctionView();
// 
// template <typename TType, typename TReturn, typename ...TArgs>
// ObjectFunctionView<TReturn(TArgs...)> MakeObjectFunctionView<TReturn(TArgs...)>(TType& aObject, TReturn(TType::*aFunctionPointer)(TArgs...))
// {
// 	return ObjectFunctionView<TReturn(TArgs...)>(aObject, aFunctionPointer);
// }
