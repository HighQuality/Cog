#pragma once

// These are wrapped in ifndef so they can be overridden via compiler settings
#ifndef PRODUCTION
#define PRODUCTION 0
#endif

#ifndef ARRAY_BOUNDSCHECK
#define ARRAY_BOUNDSCHECK !PRODUCTION
#endif

#define JSON_NOEXCEPTION

#ifdef _MSC_VER

#pragma warning ( disable : 4100 ) // Unreferenced formal parameter
#pragma warning ( disable : 4505 ) // Unreferenced local function has been removed
#pragma warning ( disable : 4714 ) // Function marked as __forceinline not inlined
#pragma warning ( disable : 4065 )

#define _HAS_EXCEPTIONS 0
#define _ITERATOR_DEBUG_LEVEL 0

#define FORCEINLINE __forceinline
#define NOINLINE __declspec(noinline)

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef _MSC_VER
#define ALIGNED(x) __declspec(align(x))
#else
#define ALIGNED(x) __attribute__ ((aligned(x)))
#endif

#else

#define FORCEINLINE inline
#define NOINLINE noinline

#endif

#include <cstdlib>
#include <iostream>
#include <string>
#include <new>
#include <initializer_list>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <malloc.h>
#include <immintrin.h>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <utility>
#include <array>

#include <Windows.h>

#undef GetObject
#undef CreateWidget
#undef DrawState
#undef GetWindowFont
#undef PlaySound
#undef CreateWindow
#undef DrawText
#undef GetMessage
#undef assert
#undef SendMessage
#undef PostMessage

#include <External/CityHash/city.h>

#include "Utilities/Defer.h"
#include "Utilities/BringIntoScope.h"

#define TimeScope Stopwatch _timeScopeWatch; defer { Println(L"%ms", _timeScopeWatch.GetElapsedTime().Milliseconds()); }

#include <TypeFundamentals/Types.h>
#include <TypeFundamentals/TypeTraits.h>

#include <Math/CogMath.h>
#include <Math/Random.h>

#include <Containers/ArrayView.h>
#include <Containers/ArraySlice.h>
#include <Containers/CogArray.h>

#include <String/CogStringView.h>
#include <String/CogStringSlice.h>
#include <String/CogString.h>

#include <Debug/DebugAsserts.h>

#include <TypeFundamentals/TypeID.h>

#include <Function/Function.h>
#include <Function/FunctionView.h>

#include <Memory/UniquePtr.h>

#define DELETE_MOVES(Type) Type(Type&&) = delete;\
	Type& operator=(Type&&) = delete;

#define DELETE_COPYCONSTRUCTORS_AND_MOVES(Type) Type(Type&&) = delete;\
	Type& operator=(Type&&) = delete;\
	Type(const Type&) = delete;\
	Type& operator=(const Type&) = delete;

#include <UtilitiesTLS.h>


#include <Images/ByteColor.h>
#include <Images/LinearColor.h>

#include <String/Format.h>

#include <Containers/Map.h>
#include <Containers/CogStack.h>

#include <Math/Vec2.h>
#include <Math/Vec4.h>

#include <Time/CogTime.h>

template <typename ...TArgs>
void Println(const TArgs& ...aArgs)
{
	String output = Format(aArgs...);
	output.Add(L'\n');
	std::wcout << output;
}

template <typename TTo, typename TFrom>
TTo CastBoundsChecked(const TFrom value)
{
	CHECK(static_cast<i64>(value) >= static_cast<i64>(MinOf<TTo>) && static_cast<i64>(value) <= static_cast<i64>(MaxOf<TTo>));
	return static_cast<TTo>(value);
}
