#pragma once

constexpr i32 MemberFunctionPointerSize = 32;

template <typename TObject, typename TFunc>
class MemberFunctionPointer;

template <typename TObject, typename TReturn, typename ...TArgs>
class MemberFunctionPointer<TObject, TReturn(TArgs...)>
{
public:
	MemberFunctionPointer()
	{
		static_assert(sizeof(MemberFunctionPointer) == MemberFunctionPointerSize, "Member Function Pointer is incorrect size");
	}

	MemberFunctionPointer(const MemberFunctionPointer&) = default;
	MemberFunctionPointer(MemberFunctionPointer&&) = default;
	
	MemberFunctionPointer& operator=(const MemberFunctionPointer&) = default;
	MemberFunctionPointer& operator=(MemberFunctionPointer&&) = default;

	~MemberFunctionPointer() = default;

	template <typename TFunction>
	FORCEINLINE MemberFunctionPointer(TFunction aFunction)
		: MemberFunctionPointer()
	{
		// If you hit an error here you're passing a const member function to a non-const Member Function Pointer,
		// change declaration from MemberFunctionPointer<const TObject, X(X)> to MemberFunctionPointer<TObject, X(X)>
		// or the other way around

		if constexpr (IsConst<TObject>)
			myFunctionConst = aFunction;
		else
			myFunction = aFunction;
	}
	
	FORCEINLINE TReturn Call(TObject& aObject, TArgs&& ...aArgs) const
	{
		return (const_cast<RemoveConst<TObject>*>(&aObject)->*myFunction)(Forward<TArgs>(aArgs)...);
	}

	bool operator==(const MemberFunctionPointer&) = delete;
	bool operator!=(const MemberFunctionPointer&) = delete;

private:
	// Makes sure all instantiations of MemberFunctionPointer are equal in size so they can be stored at a fixed size in raw memory
	union
    {
        TReturn (TObject::*myFunction)(TArgs&&...);
        TReturn (TObject::*myFunctionConst)(TArgs&&...) const;

        char buffer[MemberFunctionPointerSize];
    };

    static_assert(sizeof(buffer) >= sizeof(myFunction), "Function pointer too big, increase buffer size");
};
