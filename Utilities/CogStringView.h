#pragma once
#include "ArrayView.h"

class StringSlice;

class StringView : public ArrayView<Char>
{
public:
	using Super = ArrayView<Char>;

	FORCEINLINE StringView() = default;

	StringView(class String&&) = delete;
	StringView& operator=(class String&&) = delete;

	StringView(const StringSlice & aStringSlice);

	FORCEINLINE StringView(const ArrayView & aStringView)
		: Super(aStringView)
	{
	}

	FORCEINLINE StringView(const Char * aString)
		: Super(aString, static_cast<i32>(wcslen(aString)))
	{
	}

	FORCEINLINE bool operator==(const wchar_t * aString) const
	{
		const size_t length = wcslen(aString);
		if (length != GetLength())
			return false;
		for (i32 i = 0; i < length; ++i)
			if (this->myData[i] != aString[i])
				return false;
		return true;
	}

	FORCEINLINE bool operator!=(const wchar_t * aString) const
	{
		return !(*this == aString);
	}

	FORCEINLINE bool operator==(const StringView & aString) const
	{
		return Super::operator==(aString);
	}

	FORCEINLINE bool operator!=(const StringView & aString) const
	{
		return !(*this == aString);
	}

	bool Contains(const StringView& aString) const
	{
		if (aString.GetLength() == 0)
			return true;
		if (aString.GetLength() > GetLength())
			return false;

		const i32 substringLength = aString.GetLength();
		i32 n = this->GetLength() - aString.GetLength();
		for (i32 i=0; i<n; ++i)
		{
			if (memcmp(&this->myData[i], aString.GetData(), substringLength * sizeof Char) == 0)
				return true;
		}

		return false;
	}

	std::string ToStdString() const
	{
		std::string str;
		str.resize(GetLength());
		for (i32 i = 0; i < myLength; ++i)
			str[i] = static_cast<char>(myData[i]);
		return str;
	}

	std::wstring ToStdWString() const
	{
		return GetData();
	}

	DECLARE_ARRAY_SLICE_FUNCTIONS(StringView);


	FORCEINLINE explicit StringView(StringView && aBase, UpcastType&&)
		: Super(Move(aBase))
	{ }
};

FORCEINLINE ::std::wostream & operator<<(::std::wostream & aOut, const StringView& aString)
{
	aOut.write(aString.GetData(), aString.GetLength());
	return aOut;
}

#include "CogStringSlice.h"

FORCEINLINE StringView::StringView(const StringSlice & aStringSlice)
	: Super(aStringSlice)
{
}
