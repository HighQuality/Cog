#pragma once

class String;
class StringView;

template <typename ...TArgs>
String Format(const StringView& aFormat, const TArgs& ...aArgs);

template <typename T>
bool TrueOnFirstCallOnly(const T&)
{
	static bool FirstCall = true;
	const bool wasFirstCall = FirstCall;
	FirstCall = false;
	return wasFirstCall;
}

void EnsureLog(const char* aCondition);

#define ENSURE(condition) (!!(condition) || ((IsDebuggerPresent() && ::TrueOnFirstCallOnly([]{})) && (::EnsureLog(#condition), true) && (DebugBreak(), false)))
#define CHECK(condition) do { if (!(condition)) FATAL(L"Condition \"" #condition L"\" failed"); } while (false)

#define PRINT_ERROR(format, ...) do { String message = Format(StringView(L"%:%:\n" format L"\n"), __FILE__, __LINE__, __VA_ARGS__); OutputDebugStringW(message.GetData()); std::wcout << message; } while (false)

// #ifdef _DEBUG
#define FATAL(format, ...) do { PRINT_ERROR(format, __VA_ARGS__); DebugBreak(); abort(); } while (false)
// #else
// #define FATAL(format, ...) abort()
// #endif

#define CheckDXError(e) do { HRESULT _res = e; if (FAILED(_res)) { FATAL(L"D3D11 Expression Failed:\n%", L"" #e); } } while (false)

#ifdef _MSC_VER

#ifdef _DEBUG
#define ASSUME(e) do { if (!(e)) FATAL(L"Assumption \"%\" failed", #e); } while(false)
#define ASSUME_UNREACHABLE() FATAL(L"Unreachable assumption failed")
#else
#define ASSUME(e) __assume(e)
#define ASSUME_UNREACHABLE() __assume(0)
#endif

#define ALWAYS_ASSUME(e) __assume(e)
#define ALWAYS_ASSUME_UNREACHABLE() __assume(0)

#else

#ifdef _DEBUG
#define ASSUME(e) do { if (!(e)) FATAL(); } while(false)
#define ASSUME_UNREACHABLE() FATAL()
#else
#define ASSUME(e) do { if (!(e)) __builtin_unreachable(); } while (false)
#define ASSUME_UNREACHABLE() do { __builtin_unreachable(); } while (false)
#endif

#define ALWAYS_ASSUME(e) do { if (!(e)) __builtin_unreachable(); } while (false)
#define ALWAYS_ASSUME_UNREACHABLE() do { __builtin_unreachable(); } while (false)

#endif


// #define ASSUMEEVERYTHINGSFINE
#ifdef ASSUMEEVERYTHINGSFINE
#undef ASSUME(e)
#define ASSUME(e) __assume(e)
#undef FATAL()
#define FATAL() __assume(0)
#undef ENSURE(condition)
#define ENSURE(condition) true
#endif

template <typename TTo, typename TFrom>
FORCEINLINE TTo OverflowCheckedCast(TFrom aValue)
{
	TTo castResult = static_cast<TTo>(aValue);
	ENSURE(castResult == aValue);
	return castResult;
}
