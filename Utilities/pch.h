#pragma once

#include <iostream>
#include <string>
#include <new>
#include <initializer_list>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <malloc.h>
#include <immintrin.h>
#include <mutex>
#include <thread>
#include <condition_variable>

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

#include "CityHash/city.h"
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

#include "json.h"

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
