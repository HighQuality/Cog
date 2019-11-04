#pragma once

inline void Append(String&)
{
}

template <typename TArg>
void Append(String& aString, const TArg& aValue)
{
	aString.Append(std::to_wstring(aValue).c_str());
}

template <>
inline void Append(String& aString, const StringView& aValue)
{
	aString.Append(aValue);
}

template <>
inline void Append(String& aString, const StringSlice& aValue)
{
	aString.Append(aValue);
}

inline void Append(String& aString, const Char aCharacter)
{
	aString.Add(aCharacter);
}

inline void Append(String& aString, const char* aValue)
{
	const i32 appendLength = static_cast<i32>(strlen(aValue));
	const i32 oldLength = aString.GetLength();
	aString.Resize(oldLength + appendLength);
	for (i32 i = 0; i < appendLength; ++i)
		aString[oldLength + i] = aValue[i];
}

template <>
inline void Append(String& aString, const bool& aValue)
{
	if (static_cast<u8>(aValue) > 1)
	{
		aString.Append(L"true (");
		Append(aString, static_cast<i16>(aValue));
		aString.Add(L')');
		return;
	}
	aString.Append(aValue ? L"true" : L"false");
}

template <>
inline void Append(String& aString, const String& aValue)
{
	aString.Append(aValue);
}

template <size_t N>
inline void Append(String& aString, const wchar_t(&aValue)[N])
{
	aString.Append(aValue);
}

template <size_t N>
inline void Append(String& aString, const char(&aValue)[N])
{
	const i32 appendLength = static_cast<i32>(N);
	const i32 oldLength = aString.GetLength();
	aString.Resize(oldLength + appendLength);
	for (i32 i = 0; i < appendLength; ++i)
 		aString[oldLength + i] = aValue[i]; 
}

template <typename T>
FORCEINLINE void Append(String& aString, const ArrayView<T>& aArray)
{
	Append(aString, typeid(T).name());
	aString.Add(L'[');
	Append(aString, aArray.GetLength());
	aString.Append(L"] { ");

	for (i32 i = 0; i < aArray.GetLength(); ++i)
	{
		Append(aString, aArray[i]);

		if (i + 1 != aArray.GetLength())
			aString.Append(L", ");
		else
			aString.Add(L' ');
	}
	aString.Add(L'}');
}

template <typename T>
FORCEINLINE void Append(String& aString, const ArraySlice<T>& aArray)
{
	Append(aString, aArray.View());
}

template <typename T>
FORCEINLINE void Append(String& aString, const Array<T>& aArray)
{
	Append(aString, aArray.View());
}

template <typename TArg, typename ...TArgs>
void Append(String& aString, const TArg& aArg, const TArgs& ...aArgs)
{
	Append(aString, aArg);
	Append(aString, aArgs...);
}

template <typename ...TArgs>
String Concatenate(const TArgs& ...aArgs)
{
	String str;
	Append(str, aArgs...);
	return str;
}
