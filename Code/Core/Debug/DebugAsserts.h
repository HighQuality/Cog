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

void EnsureLog(StringView aMessage);
void ErrorLog(StringView aMessage);

#define ENSURE_MSG(condition, format, ...) \
	( \
		!!(condition) \
		|| \
		( \
			( \
				::TrueOnFirstCallOnly([]{}) \
			) \
			&& \
			( \
				::EnsureLog(Format(L"Ensure condition failed on " __FILE__ L":%:\n" format, __LINE__, __VA_ARGS__).View()) \
				, true \
			) \
			&& \
			( \
				IsDebuggerPresent() \
			) \
			&& \
			( \
				(DebugBreak(), false) \
			) \
		) \
	)

#define ENSURE(condition) ENSURE_MSG(condition, #condition)

#define CHECK_MSG(condition, format, ...) do { if (!(condition)) FATAL(format, __VA_ARGS__); } while (false)
#define CHECK(condition) do { if (!(condition)) FATAL(L"Condition \"" #condition L"\" failed"); } while (false)

#define PRINT_ERROR(format, ...) do { String _fatalmessage = Format(StringView(L"%:% (%):\n" format L"\n"), __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); ErrorLog(_fatalmessage); } while (false)

// #ifdef _DEBUG
#define FATAL(format, ...) do { PRINT_ERROR(format, __VA_ARGS__); DebugBreak(); abort(); } while (false)
#define FATAL_PURE_VIRTUAL() do { FATAL(L"Pure virtual function called"); } while (false)
// #else
// #define FATAL(format, ...) abort()
// #endif

#define TODO do { FATAL(L"Function not implemented"); } while (false)

#ifdef _MSC_VER

#ifdef _DEBUG
#define ASSUME(e) do { if (!(e)) FATAL(L"Assumption \"%\" failed", #e); } while (false)
#define ASSUME_UNREACHABLE() FATAL(L"Unreachable assumption failed")
#define CHECK_PEDANTIC(e) CHECK(e)
#else
#define ASSUME(e) __assume(e)
#define ASSUME_UNREACHABLE() __assume(0)
#define CHECK_PEDANTIC(e) do { } while (false)
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

#include <String/CogStringView.h>

template <typename TTo, typename TFrom>
FORCEINLINE TTo OverflowCheckedCast(TFrom aValue)
{
	TTo castResult = static_cast<TTo>(aValue);
	ENSURE(castResult == aValue);
	return castResult;
}
