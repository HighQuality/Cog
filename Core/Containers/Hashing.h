#pragma once
#include <String/CogString.h>

template <typename T>
FORCEINLINE size_t HashOf(const T & aValue)
{
	return std::hash<T>()(aValue);
}

template<>
FORCEINLINE size_t HashOf<StringView>(const StringView & aValue)
{
	return CityHash64(reinterpret_cast<const char*>(aValue.GetData()), aValue.GetLength() * sizeof Char);
}

template<>
FORCEINLINE size_t HashOf<StringSlice>(const StringSlice & aValue)
{
	return CityHash64(reinterpret_cast<const char*>(aValue.GetData()), aValue.GetLength() * sizeof Char);
}

template<>
FORCEINLINE size_t HashOf<String>(const String & aValue)
{
	return CityHash64(reinterpret_cast<const char*>(aValue.GetData()), aValue.GetLength() * sizeof Char);
}
