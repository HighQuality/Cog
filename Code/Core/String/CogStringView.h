#pragma once

class String;
class StringSlice;

class StringView : public ArrayView<Char>
{
public:
	using Super = ArrayView<Char>;

	FORCEINLINE StringView() = default;

	StringView(class String&&) = delete;
	StringView& operator=(class String&&) = delete;

	StringView(const StringSlice& aStringSlice);

	FORCEINLINE StringView(const ArrayView& aStringView)
		: Super(aStringView)
	{
		if (GetLength() > 0 && Last() == L'\0')
			--this->myLength;
	}

	FORCEINLINE StringView(const Char* aString)
		: Super(aString, static_cast<i32>(wcslen(aString)))
	{
	}
	
	// These should not be necessary due to implicit conversion from const wchar_t* to StringView
	//
	// FORCEINLINE bool operator==(const wchar_t* aString) const
	// {
	// 	const size_t length = wcslen(aString);
	// 	if (length != GetLength())
	// 		return false;
	// 	for (i32 i = 0; i < length; ++i)
	// 		if (this->myData[i] != aString[i])
	// 			return false;
	// 	return true;
	// }
	// 
	// FORCEINLINE bool operator!=(const wchar_t* aString) const
	// {
	// 	return !(*this == aString);
	// }

	FORCEINLINE bool operator==(const StringView& aString) const
	{
		return Super::operator==(aString);
	}

	FORCEINLINE bool operator!=(const StringView& aString) const
	{
		return !(*this == aString);
	}

	String ToLower() const;

	String ToUpper() const;

	bool Contains(const StringView& aString) const
	{
		if (aString.GetLength() == 0)
			return true;
		if (aString.GetLength() > GetLength())
			return false;

		const i32 substringLength = aString.GetLength();
		i32 n = this->GetLength() - aString.GetLength();
		for (i32 i = 0; i < n; ++i)
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
		return std::wstring(GetData(), GetLength());
	}

	void TrimLeft()
	{
		if (GetLength() == 0)
			return;

		const i32 length = GetLength();

		for (i32 i = 0; i < length; ++i)
		{
			if (!iswspace((*this)[i]))
			{
				*this = ChopFromStart(i);
				break;
			}
		}
	}

	void TrimRight()
	{
		if (GetLength() == 0)
			return;

		for (i32 i = GetLength() - 1; i >= 0; --i)
		{
			if (!iswspace((*this)[i]))
			{
				*this = SliceFromStart(i + 1);
				break;
			}
		}
	}

	void Trim()
	{
		TrimLeft();
		TrimRight();
	}

	StringView GetTrimmedLeft() const
	{
		StringView copy = *this;
		copy.TrimLeft();
		return copy;
	}

	StringView GetTrimmedRight() const
	{
		StringView copy = *this;
		copy.TrimRight();
		return copy;
	}

	StringView GetTrimmed() const
	{
		StringView copy = *this;
		copy.Trim();
		return copy;
	}

	bool StartsWith(const StringView& aArray) const { return ArrayView<Char>::StartsWith(aArray); }

	bool EndsWith(const StringView& aArray) const { return ArrayView<Char>::EndsWith(aArray); }

	FORCEINLINE explicit operator bool() const { return GetLength() > 0; }

	DECLARE_ARRAY_SLICE_FUNCTIONS(StringView);


	FORCEINLINE explicit StringView(StringView&& aBase, UpcastType&&)
		: Super(Move(aBase))
	{ }
};

FORCEINLINE::std::wostream& operator<<(::std::wostream& aOut, const StringView& aString)
{
	aOut.write(aString.GetData(), aString.GetLength());
	return aOut;
}

#include "CogStringSlice.h"

FORCEINLINE StringView::StringView(const StringSlice& aStringSlice)
	: Super(aStringSlice)
{
}

#include "CogString.h"

inline String StringView::ToLower() const
{
	String str;
	const i32 length = GetLength();
	str.Resize(length);
	for (i32 i = 0; i < length; ++i)
		str[i] = towlower((*this)[i]);
	return str;
}

inline String StringView::ToUpper() const
{
	String str;
	const i32 length = GetLength();
	str.Resize(length);
	for (i32 i = 0; i < length; ++i)
		str[i] = towupper((*this)[i]);
	return str;
}
