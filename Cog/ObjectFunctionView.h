#pragma once
#include "Pointer.h"

template <typename TFunc>
class ObjectFunctionView;

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

	explicit operator bool() const { return IsValid(); }

	bool IsValid() const
	{
		if (!myIsValid)
			return false;
		return myIsValid(*this);
	}

	template <typename TCallRet = TReturn>
	EnableIf<!IsSame<TCallRet, void>, bool> TryCall(const TArgs& ...aArgs, TCallRet& aReturnValue) const
	{
		if (!IsValid())
			return false;
		aReturnValue = (*myFunctionCaller)(*this, aArgs...);
		return true;
	}

	template <typename TCallRet = TReturn>
	EnableIf<IsSame<TCallRet, void>, bool> TryCall(const TArgs& ...aArgs) const
	{
		if (!IsValid())
			return false;
		(*myFunctionCaller)(*this, aArgs...);
		return true;
	}

	TReturn Call(const TArgs& ...aArgs) const
	{
		CHECK(IsValid());
		return (*myFunctionCaller)(*this, aArgs...);
	}

private:
	template <typename TObject, typename TMemberFunction>
	void Initialize(TObject& aObject, TMemberFunction aMemberFunction)
	{
		new (static_cast<void*>(&myPointer)) Ptr<TObject>(aObject);

		auto lambda = [aMemberFunction](TObject& aObject, const TArgs&... aArgs)
		{
			return (aObject.*aMemberFunction)(aArgs...);
		};

		myFunctionCaller = [](const ObjectFunctionView& aSelf, const TArgs& ...aArgs)
		{
			const Ptr<TObject>& ptr = *reinterpret_cast<const Ptr<TObject>*>(&aSelf.myPointer);
			return reinterpret_cast<const decltype(lambda)*>(&aSelf.myFunctionStorage)->operator()(*ptr, aArgs...);
		};

		myIsValid = [](const ObjectFunctionView& aSelf)
		{
			const Ptr<TObject>& ptr = *reinterpret_cast<const Ptr<TObject>*>(&aSelf.myPointer);
			return ptr.IsValid();
		};

		new(static_cast<void*>(&myFunctionStorage)) decltype(lambda)(Move(lambda));
	}

	std::aligned_storage_t<sizeof(Ptr<Entity>), alignof(Ptr<Entity>)> myPointer;
	std::aligned_storage_t<8> myFunctionStorage;
	TReturn (*myFunctionCaller)(const ObjectFunctionView&, const TArgs& ...) = nullptr;
	bool (*myIsValid)(const ObjectFunctionView&) = nullptr;
};

// template <typename TType, typename TFunc>
// void MakeObjectFunctionView();
// 
// template <typename TType, typename TReturn, typename ...TArgs>
// ObjectFunctionView<TReturn(TArgs...)> MakeObjectFunctionView<TReturn(TArgs...)>(TType& aObject, TReturn(TType::*aFunctionPointer)(TArgs...))
// {
// 	return ObjectFunctionView<TReturn(TArgs...)>(aObject, aFunctionPointer);
// }
