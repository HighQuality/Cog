#pragma once

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

#pragma warning ( disable : 4100 ) // Unreferenced formal parameter
#pragma warning ( disable : 4505 ) // Unreferenced local function has been removed
#pragma warning ( disable : 4714 ) // Function marked as __forceinline not inlined

#include "CityHash/city.h"

#ifdef _MSC_VER
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE inline
#endif

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

#define DELETE_MOVES(Type) Type(Type&&) = delete; \
	Type& operator=(Type&&) = delete;

#include "Types.h"

#include "Random.h"
#include "DebugAsserts.h"
#include "CogMath.h"

#include "CogArray.h"
#include "ArraySlice.h"
#include "ArrayView.h"

#include "CogString.h"
#include "CogStringSlice.h"
#include "CogStringView.h"

#include "Format.h"

#include "Map.h"

#include "Vec2.h"
#include "Vec4.h"

#include "CogTime.h"

#include "LinearColor.h"

template <typename ...TArgs>
void Println(const TArgs& ...aArgs)
{
	String output = Format(aArgs...);
	output.Append(L"\r\n");
	std::wcout << output;
	OutputDebugStringW(output.GetData());
}

template <typename TTo, typename TFrom>
TTo CastBoundsChecked(const TFrom value)
{
	CHECK(static_cast<i64>(value) >= static_cast<i64>(MinOf<TTo>) && static_cast<i64>(value) <= static_cast<i64>(MaxOf<TTo>));
	return static_cast<TTo>(value);
}
