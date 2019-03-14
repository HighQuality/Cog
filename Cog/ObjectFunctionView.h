#pragma once
#include "Pointer.h"
#include <InlineVirtualObject.h>
#include <MemberFunctionPointer.h>

template <typename TFunc>
class ObjectFunctionView;

template <typename TReturn, typename ...TArgs>
class ObjectFunctionView<TReturn(TArgs ...)>
{
public:
	ObjectFunctionView() = default;

	template <typename TObject>
	FORCEINLINE ObjectFunctionView(TObject& aObject, TReturn(TObject::*aFunction)(TArgs...))
	{
		myObject = &aObject;
		using FunctionType = MemberFunctionPointer<TObject, TReturn(TArgs...)>;
		new (static_cast<void*>(&myMemberFunctionPointerStorage)) FunctionType(aFunction);

		myFunctionCaller = [](const ObjectFunctionView& This, TArgs ...aArgs) -> TReturn
		{
			const auto& storage = *reinterpret_cast<const FunctionType*>(&This.myMemberFunctionPointerStorage);
			return storage.Call(*static_cast<TObject*>(This.myObject), std::forward<TArgs>(aArgs)...);
		};

		static_assert(std::is_trivially_destructible_v<MemberFunctionPointer<TObject, TReturn(TArgs...)>>, "Member function storage must be trivially destructible in order not to leak");
	}

	template <typename TObject>
	FORCEINLINE ObjectFunctionView(const TObject& aObject, TReturn(TObject::*aFunction)(TArgs...) const)
	{
		myObject = const_cast<TObject*>(&aObject);
		using FunctionType = MemberFunctionPointer<const TObject, TReturn(TArgs...)>;
		new (static_cast<void*>(&myMemberFunctionPointerStorage)) FunctionType(aFunction);

		myFunctionCaller = [](const ObjectFunctionView& This, TArgs ...aArgs) -> TReturn
		{
			const auto& storage = *reinterpret_cast<const FunctionType*>(&This.myMemberFunctionPointerStorage);
			return storage.Call(*static_cast<TObject*>(This.myObject), std::forward<TArgs>(aArgs)...);
		};

		static_assert(std::is_trivially_destructible_v<MemberFunctionPointer<const TObject, TReturn(TArgs...)>>, "Member function storage must be trivially destructible in order not to leak");
	}
	
	FORCEINLINE explicit operator bool() const { return IsValid(); }

	FORCEINLINE bool IsValid() const
	{
		return myObject;
	}

	template <typename TCallRet = TReturn>
	FORCEINLINE EnableIf<!IsSame<TCallRet, void>, bool> TryCall(TArgs ...aArgs, TCallRet& aReturnValue) const
	{
		if (!IsValid())
			return false;
		aReturnValue = Call(std::forward(aArgs)...);
		return true;
	}

	template <typename TCallRet = TReturn>
	FORCEINLINE EnableIf<IsSame<TCallRet, void>, bool> TryCall(TArgs ...aArgs) const
	{
		if (!IsValid())
			return false;
		Call(std::forward(aArgs)...);
		return true;
	}

	FORCEINLINE TReturn Call(TArgs ...aArgs) const
	{
		CHECK(IsValid());
		return myFunctionCaller(*this, std::forward<TArgs>(aArgs)...);
	}

private:
	std::aligned_storage_t<MemberFunctionPointerSize> myMemberFunctionPointerStorage;
	void* myObject;
	TReturn(*myFunctionCaller)(const ObjectFunctionView&, TArgs...);
};
