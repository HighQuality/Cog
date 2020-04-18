#pragma once
#include "Types.h"

template <typename T>
struct RemoveReferenceImpl { using Type = T; };
template <typename T>
struct RemoveReferenceImpl<T&> { using Type = T; };
template <typename T>
struct RemoveReferenceImpl<T&&> { using Type = T; };

template <typename T>
using RemoveReference = typename RemoveReferenceImpl<T>::Type;

template <typename T>
struct RemoveConstImpl { using Type = T; };
template <typename T>
struct RemoveConstImpl<const T> { using Type = T; };

template <typename T>
using RemoveConst = typename RemoveConstImpl<T>::Type;

template <typename T>
struct RemovePointerImpl { using Type = T; };
template <typename T>
struct RemovePointerImpl<T*> { using Type = T; };

template <typename T>
using RemovePointer = typename RemovePointerImpl<T>::Type;

template <typename T>
struct AddPointerImpl { using Type = T*; };
template <typename T>
struct AddPointerImpl<T*> { using Type = T*; };

template <typename T>
using AddPointer = typename AddPointerImpl<T>::Type;

template <typename T>
using ReferenceOf = T&;
template <typename T>
using ConstReferenceOf = const T&;

template <typename T>
using PointerOf = T*;
template <typename T>
using ConstPointerOf = const T*;

template <typename T>
struct IsConstImpl { static constexpr bool Value = false; };
template <typename T>
struct IsConstImpl<const T> { static constexpr bool Value = true; };

template <typename T>
constexpr bool IsConst = IsConstImpl<T>::Value;

template <typename T1, typename T2>
struct IsSameImpl { static constexpr bool Value = false; };
template <typename T>
struct IsSameImpl<T, T> { static constexpr bool Value = true; };

template <typename T1, typename T2>
constexpr bool IsSame = IsSameImpl<T1, T2>::Value;

template <typename T>
struct IsCompleteImpl
{
	typedef char No;
	struct Yes { char Dummy[2]; };

	template <typename T2, typename = decltype(sizeof(std::declval<T2>())) >
	static Yes Check(T2*);

	template <class U>
	static No Check(...);

	static constexpr bool Value = sizeof(Check<T>(nullptr)) == sizeof(Yes);
};

template <typename T>
constexpr bool IsComplete = IsCompleteImpl<T>::Value;

template <bool Condition, typename T1, typename T2>
struct SelectImpl { using Type = T2; };
template <typename T1, typename T2>
struct SelectImpl<true, T1, T2> { using Type = T1; };

template <bool Condition, typename T1, typename T2>
using Select = typename SelectImpl<Condition, T1, T2>::Type;

template <typename T>
struct IsReferenceImpl { static constexpr bool Value = false; };
template <typename T>
struct IsReferenceImpl<T&> { static constexpr bool Value = true; };
template <typename T>
struct IsReferenceImpl<T&&> { static constexpr bool Value = true; };

template <typename T>
constexpr bool IsReference = IsReferenceImpl<T>::Value;

template <typename T>
struct IsPointerImpl { static constexpr bool Value = false; };

template <typename T>
struct IsPointerImpl<T*> { static constexpr bool Value = true; };

template <typename T>
struct IsPointerImpl<const T*> { static constexpr bool Value = true; };

template <typename T>
constexpr bool IsPointer = IsPointerImpl<T>::Value;

template <typename T>
struct IsLValueReferenceImpl { static constexpr bool Value = false; };
template <typename T>
struct IsLValueReferenceImpl<T&> { static constexpr bool Value = true; };

template <typename T>
constexpr bool IsLValueReference = IsLValueReferenceImpl<T>::Value;

template <typename T>
struct IsFunctionImpl { static constexpr bool Value = false; };
template <typename TRet, typename... TArgs>
struct IsFunctionImpl<TRet(TArgs...)> { static constexpr bool Value = true; };

template <typename T>
constexpr bool IsFunction = IsFunctionImpl<T>::Value;

template <bool TEnable, typename TType = void>
struct EnableIfImpl {};
template <typename TType>
struct EnableIfImpl<true, TType> { using Type = TType; };

template <bool TEnable, typename TType = void>
using EnableIf = typename EnableIfImpl<TEnable, TType>::Type;

template <typename T>
struct IsIntegerImpl { static constexpr bool Value = false; };
template <>
struct IsIntegerImpl<i8> { static constexpr bool Value = true; };
template <>
struct IsIntegerImpl<u8> { static constexpr bool Value = true; };
template <>
struct IsIntegerImpl<i16> { static constexpr bool Value = true; };
template <>
struct IsIntegerImpl<u16> { static constexpr bool Value = true; };
template <>
struct IsIntegerImpl<i32> { static constexpr bool Value = true; };
template <>
struct IsIntegerImpl<u32> { static constexpr bool Value = true; };
template <>
struct IsIntegerImpl<i64> { static constexpr bool Value = true; };
template <>
struct IsIntegerImpl<u64> { static constexpr bool Value = true; };

template <>
struct IsIntegerImpl<char> { static constexpr bool Value = true; };
template <>
struct IsIntegerImpl<wchar_t> { static constexpr bool Value = true; };

template <typename T>
struct IsIntegerImpl<const T> { static constexpr bool Value = IsIntegerImpl<T>::Value; };
template <typename T>
struct IsIntegerImpl<volatile T> { static constexpr bool Value = IsIntegerImpl<T>::Value; };
template <typename T>
struct IsIntegerImpl<const volatile T> { static constexpr bool Value = IsIntegerImpl<T>::Value; };

template <typename T>
constexpr bool IsInteger = IsIntegerImpl<T>::Value;

template <typename T>
struct IsFloatingPointImpl { static constexpr bool Value = false; };
template <>
struct IsFloatingPointImpl<f32> { static constexpr bool Value = true; };
template <>
struct IsFloatingPointImpl<f64> { static constexpr bool Value = true; };

template <typename T>
struct IsFloatingPointImpl<const T> { static constexpr bool Value = IsFloatingPointImpl<T>::Value; };
template <typename T>
struct IsFloatingPointImpl<volatile T> { static constexpr bool Value = IsFloatingPointImpl<T>::Value; };
template <typename T>
struct IsFloatingPointImpl<const volatile T> { static constexpr bool Value = IsFloatingPointImpl<T>::Value; };

template <typename T>
constexpr bool IsFloatingPoint = IsFloatingPointImpl<T>::Value;

template <typename T>
constexpr bool IsNumber = IsInteger<T> || IsFloatingPoint<T>;

template <typename TDerived, typename TBase>
struct IsDerivedFromImpl
{
	using Yes = char[1];
	using No = char[2];
	static Yes& Test(RemoveConst<TBase>*);
	static Yes& Test(const RemoveConst<TBase>*);
	static No& Test(...);
};

template <typename TDerived, typename TBase>
constexpr bool IsDerivedFrom = sizeof(IsDerivedFromImpl<TDerived, TBase>::Test(static_cast<TDerived*>(nullptr))) == 1;

template <typename T>
RemoveReference<T>&& Move(const T& Object) = delete;

template <typename T>
FORCEINLINE RemoveReference<T>&& Move(T&& Object)
{
	return static_cast<RemoveReference<T>&&>(Object);
}

template <typename T>
FORCEINLINE T&& Forward(RemoveReference<T>& Object)
{
	return static_cast<T&&>(Object);
}

template <typename T>
constexpr T MaxOf = std::numeric_limits<T>::max();

template <typename T>
constexpr T MinOf = std::numeric_limits<T>::min();

template <typename T>
struct LessThan
{
	FORCEINLINE static bool Compare(const T& aA, const T& aB)
	{
		return aA < aB;
	}

	FORCEINLINE bool operator()(const T& aA, const T& aB) const
	{
		return Compare(aA, aB);
	}
};

template <typename T>
struct GreaterThanEquals
{
	FORCEINLINE static bool Compare(const T& aA, const T& aB)
	{
		return !LessThan<T>::Compare(aA, aB);
	}

	FORCEINLINE bool operator()(const T& aA, const T& aB) const
	{
		return Compare(aA, aB);
	}
};

template <typename T>
struct Equals
{
	FORCEINLINE static bool Compare(const T& aA, const T& aB)
	{
		return !(LessThan<T>::Compare(aA, aB) || LessThan<T>::Compare(aB, aA));
	}

	FORCEINLINE bool operator()(const T& aA, const T& aB) const
	{
		return Compare(aA, aB);
	}
};

template <typename T>
struct GreaterThan
{
	FORCEINLINE static bool Compare(const T& aA, const T& aB)
	{
		return !(LessThan<T>::Compare(aA, aB) || Equals<T>::Compare(aA, aB));
	}

	FORCEINLINE bool operator()(const T& aA, const T& aB) const
	{
		return Compare(aA, aB);
	}
};

template <typename T>
struct LessThanEquals
{
	FORCEINLINE static bool Compare(const T& aA, const T& aB)
	{
		return !GreaterThan<T>::Compare(aA, aB);
	}

	FORCEINLINE bool operator()(const T& aA, const T& aB) const
	{
		return Compare(aA, aB);
	}
};
