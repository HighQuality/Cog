#pragma once
#include "Containers/CogArray.h"
#include "CogStringView.h"

class String : public Array<Char, ArrayHeapAllocator<ArraySlice<Char>, Char>, true>
{
public:
	using Super = Array<Char, ArrayHeapAllocator<ArraySlice<Char>, Char>, true>;

	FORCEINLINE String() = default;

	FORCEINLINE explicit String(const StringView& aString)
		: Super()
	{
		this->Resize(aString.GetLength());
		memcpy(this->myData, aString.GetData(), sizeof Char * GetLength());
	}

	FORCEINLINE explicit String(const Char* aString)
		: Super()
	{
		this->Resize(static_cast<i32>(wcslen(aString)));
		memcpy(this->myData, aString, sizeof Char * GetLength());
	}

	FORCEINLINE explicit String(const char* aString)
		: Super()
	{
		this->Resize(static_cast<i32>(strlen(aString)));
		for (i32 i=0; i<this->myLength; ++i)
			this->myData[i] = aString[i];
	}

	FORCEINLINE String & operator=(const StringView & aString)
	{
		this->Resize(aString.GetLength());
		memcpy(this->myData, aString.GetData(), sizeof Char * GetLength());
		return *this;
	}

	FORCEINLINE String & operator=(const Char * aString)
	{
		this->Resize(static_cast<i32>(wcslen(aString)));
		memcpy(this->myData, aString, sizeof Char * GetLength());
		return *this;
	}

	FORCEINLINE String(String && aMove) noexcept
		: Super(Move(aMove))
	{
	}

	FORCEINLINE String& operator=(String && aMove) noexcept
	{
		Super::operator=(Move(aMove));
		return *this;
	}

	FORCEINLINE void CheckEndsWithZero() const
	{
		if (!this->myData[this->myLength] == L'\0')
			abort();
	}

	FORCEINLINE explicit String(const String&) = default;
	FORCEINLINE String& operator=(const String&) = delete;
	FORCEINLINE ~String() = default;

	FORCEINLINE StringView View() const
	{
		return *this;
	}

	void Append(const StringView& aString)
	{
		Super::Append(aString);
	}

	explicit operator std::wstring() const
	{
		return std::wstring(GetData(), GetLength());
	}

	explicit operator std::string() const
	{
		std::string string;
		string.reserve(GetLength());
		for (const Char character : *this)
			string.push_back(static_cast<char>(character));
		return string;
	}

	DECLARE_ARRAY_SLICE_FUNCTIONS(StringSlice);

};

FORCEINLINE::std::wostream & operator<<(::std::wostream & aOut, const String& aString)
{
	aOut.write(aString.GetData(), aString.GetLength());
	return aOut;
}
