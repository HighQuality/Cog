#pragma once

#pragma warning ( disable : 4065 )

#include <iostream>
#include <string>
#include <new>
#include <initializer_list>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <malloc.h>
#include <immintrin.h>

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

#define ARRAY_BOUNDSCHECK 1

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

#include "CogTime.h"

// 0 = unassigned, 1 = game thread
inline u16 gThreadID = 0;

FORCEINLINE bool IsInGameThread()
{
	return gThreadID == 1;
}

template <typename ...TArgs>
void Println(const TArgs& ...aArgs)
{
	String output = Format(aArgs...);
	output.Append(L"\r\n");
	std::wcout << output;
	OutputDebugStringW(output.GetData());
}

template <typename TTo, typename TFrom>
TTo CastBoundsChecked(const TFrom from)
{
	CHECK(from >= MinOf<TTo> && from <= MaxOf<TTo>);
	return static_cast<TTo>(from);
}
