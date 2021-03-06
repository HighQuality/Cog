#pragma once

#if _WIN32 || _WIN64
#if _WIN64
#define ENV64
#else
#define ENV32
#endif
#endif

#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENV64
#else
#define ENV32
#endif
#endif

#define _CRT_SECURE_NO_WARNINGS

#define STRINGIFY_INNER(x) #x
#define STRINGIFY(x) STRINGIFY_INNER(x)

#define JOIN_INNER(a, b) a ## b
#define JOIN(a, b) JOIN_INNER(a, b)

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
#pragma warning ( disable : 4702 ) // Unreachable code

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
#include <type_traits>

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

#include "Utilities/Overloaded.h"
#include "Utilities/Defer.h"
#include "Utilities/BringIntoScope.h"

#define TimeScope Stopwatch _timeScopeWatch; defer { Println(L"%ms", _timeScopeWatch.GetElapsedTime().Milliseconds()); }

#define DELETE_MOVES(Type) Type(Type&&) = delete;\
	Type& operator=(Type&&) = delete;

#define DELETE_COPYCONSTRUCTORS_AND_MOVES(Type) Type(Type&&) = delete;\
	Type& operator=(Type&&) = delete;\
	Type(const Type&) = delete;\
	Type& operator=(const Type&) = delete;

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

#include "CoreTLS.h"

#include <Memory/UniquePtr.h>

#include <Images/ByteColor.h>
#include <Images/LinearColor.h>

#include <String/Format.h>

#include <Containers/Map.h>
#include <Containers/CogStack.h>

#include <Math/Vec2.h>
#include <Math/Vec4.h>

#include <Time/TimeSpan.h>

#include <Utilities/CommandLine.h>

template <typename ...TArgs>
void Println(const TArgs& ...aArgs)
{
	String output = Format(aArgs...);
	output.Add(L'\n');
	std::wcout << output;

	if (IsDebuggerPresent())
		OutputDebugStringW(output.GetData());
}

template <typename TTo, typename TFrom>
TTo CastBoundsChecked(const TFrom value)
{
	CHECK(static_cast<i64>(value) >= static_cast<i64>(MinOf<TTo>) && static_cast<i64>(value) <= static_cast<i64>(MaxOf<TTo>));
	return static_cast<TTo>(value);
}
