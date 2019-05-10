#pragma once

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

// When the header tools comes by this it stops parsing the rest of the header, mainly useful for large third party headers that we do not need to consider for code generation
#define COG_IGNORE_HEADER

#ifdef _MSC_VER
#pragma warning ( disable : 4100 ) // Unreferenced formal parameter
#pragma warning ( disable : 4505 ) // Unreferenced local function has been removed
#pragma warning ( disable : 4714 ) // Function marked as __forceinline not inlined
#pragma warning ( disable : 4065 )
#endif

#include <Utilities/External/CityHash/city.h>

#ifdef _MSC_VER
#define FORCEINLINE __forceinline
#define NOINLINE __declspec(noinline)
#else
#define FORCEINLINE inline
#define NOINLINE noinline
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
#undef PostMessage

#define DELETE_MOVES(Type) Type(Type&&) = delete;\
	Type& operator=(Type&&) = delete;

#define DELETE_COPYCONSTRUCTORS_AND_MOVES(Type) Type(Type&&) = delete;\
	Type& operator=(Type&&) = delete;\
	Type(const Type&) = delete;\
	Type& operator=(const Type&) = delete;

#include <Utilities/Types.h>

using ChunkIndex = u8;

#define COGTYPE(...)
#define GENERATED_BODY_IMPL_INNER(HeaderFile, Line) GENERATED_BODY_ ## HeaderFile ## Line
#define GENERATED_BODY_IMPL(HeaderFile, Line) GENERATED_BODY_IMPL_INNER(HeaderFile, Line)
#define GENERATED_BODY() GENERATED_BODY_IMPL(CURRENT_HEADER_FILE, __LINE__)

#include <Utilities/UtilitiesTLS.h>

#include <Utilities/Random.h>
#include <Utilities/DebugAsserts.h>
#include <Utilities/CogMath.h>

#include <Utilities/CogArray.h>
#include <Utilities/ArraySlice.h>
#include <Utilities/ArrayView.h>

#include <Utilities/CogString.h>
#include <Utilities/CogStringSlice.h>
#include <Utilities/CogStringView.h>

#include <Utilities/Format.h>

#include <Utilities/Map.h>

#include <Utilities/Vec2.h>
#include <Utilities/Vec4.h>

#include <Utilities/CogTime.h>

#include <Utilities/LinearColor.h>

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

// These are wrapped in ifndef so they can be overriden via compiler settings
#ifndef PRODUCTION
#define PRODUCTION 0
#endif

#ifndef ARRAY_BOUNDSCHECK
#define ARRAY_BOUNDSCHECK 1
#endif

#define WITH_CLIENT 1

bool IsInGameThread();

#define CHECK_COMPONENT_INCLUDED(Type) do { static_assert(IsComplete<Type>, "Component header must be included"); } while (false)

#include <Cog/Casts.h>
#include <Cog/Entity.h>
#include <Cog/EntityInitializer.h>
#include <Cog/Component.h>
#include <Cog/Pointer.h>
#include <Cog/FrameData.h>

#if WITH_CLIENT

#include <d3d11_1.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

void SetResourceDebugName(ID3D11DeviceChild* resource, StringView aDebugName);

#define CheckDXError(e) do { HRESULT _res = e; if (FAILED(_res)) { FATAL(L"D3D11 Expression Failed:\n%", L"" #e); } } while (false)

#endif // WITH_CLIENT
