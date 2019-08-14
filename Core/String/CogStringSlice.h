#pragma once
#include "ContainerS/ArraySlice.h"
#include "CogStringView.h"

class String;

class StringSlice : public ArraySlice<Char>
{
public:
	using Super = ArraySlice<Char>;

	StringSlice(class String&&) = delete;
	StringSlice& operator=(class String&&) = delete;

	FORCEINLINE StringSlice(const ArraySlice<Char> & aSlice)
		: Super(aSlice)
	{
	}

	DECLARE_ARRAY_SLICE_FUNCTIONS(StringSlice)

	FORCEINLINE explicit StringSlice(ArraySlice<Char> && aBase, UpcastType&&)
		: Super(Move(aBase))
	{ }

	FORCEINLINE bool operator==(const wchar_t* aString) const
	{
		return StringView(*this) == aString;
	}

	FORCEINLINE bool operator!=(const wchar_t* aString) const
	{
		return !(*this == aString);
	}

	FORCEINLINE bool operator==(const StringView& aString) const
	{
		return StringView(*this) == aString;
	}

	FORCEINLINE bool operator!=(const StringView& aString) const
	{
		return !(*this == aString);
	}
};

FORCEINLINE bool operator==(const StringView& aLeft, const StringSlice& aRight)
{
	return aRight == aLeft;
}

FORCEINLINE bool operator!=(const StringView& aLeft, const StringSlice& aRight)
{
	return aRight != aLeft;
}


FORCEINLINE::std::wostream & operator<<(::std::wostream & aOut, const StringSlice& aString)
{
	aOut.write(aString.GetData(), aString.GetLength());
	return aOut;
}
