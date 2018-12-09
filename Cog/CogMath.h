#pragma once
#include "Types.h"

constexpr f32 Pi = 3.14159265358979323846f;
constexpr f32 TwoPi = Pi * 2.f;
constexpr f32 HalfPi = Pi / 2.f;
constexpr f32 QuarterPi = Pi / 4.f;
constexpr f32 InvPi = 1.f / Pi;

constexpr f32 Tau = TwoPi;

template <typename T>
T Min(T aFirst, T aSecond)
{
	return aFirst < aSecond ? aFirst : aSecond;
}

template <typename T>
T Max(T aFirst, T aSecond)
{
	return aFirst < aSecond ? aSecond : aFirst;
}

template <typename T>
T Clamp(T aValue, T aMin, T aMax)
{
	return Max(aMin, Min(aMax, aValue));
}

template <typename T>
T Abs(T aValue)
{
	return aValue >= T(0) ? aValue : -aValue;
}

FORCEINLINE f32 FloorToFloat(f32 aValue)
{
	return floorf(aValue);
}

FORCEINLINE i32 FloorToInt(f32 aValue)
{
	return static_cast<i32>(FloorToFloat(aValue));
}

FORCEINLINE i32 TruncToInt(f32 aValue)
{
	return static_cast<i32>(aValue);
}

FORCEINLINE f32 TruncToFloat(f32 aValue)
{
	return static_cast<f32>(TruncToInt(aValue));
}

template <typename T>
void Swap(T& aFirst, T& aSecond)
{
	T temp = Move(aSecond);
	aSecond = Move(aFirst);
	aFirst = Move(temp);
}

inline u32 FloorLog2(u32 aValue)
{
	u32 pos = 0;

	if (aValue >= 1 << 16)
	{
		aValue >>= 16;
		pos += 16;
	}
	if (aValue >= 1 << 8)
	{
		aValue >>= 8; pos += 8;
	}
	if (aValue >= 1 << 4)
	{
		aValue >>= 4; pos += 4;
	}
	if (aValue >= 1 << 2)
	{
		aValue >>= 2; pos += 2;
	}
	if (aValue >= 1 << 1)
	{
		pos += 1;
	}

	return (aValue == 0) ? 0 : pos;
}

inline u32 CountLeadingZeros(const u32 aValue)
{
	if (aValue == 0)
		return 32;
	return 32 - FloorLog2(aValue);
}

inline u32 CeilToLogTwo(const u32 aValue)
{
	const i32 bitmask = (static_cast<i32>(CountLeadingZeros(aValue) << 26)) >> 31;
	return ((32 - CountLeadingZeros(aValue - 1)) & (~bitmask));
}

inline u32 CeilToPowerTwo(const u32 aValue)
{
	return 1 << (CeilToLogTwo(aValue) + 1);
}

inline f32 Sqrt(const f32 aValue)
{
	return sqrtf(aValue);
}

#include "Angle.h"

inline void SinCos(const Angle aAngle, f32& aSin, f32& aCos)
{
	const f32 radian = aAngle.AsRadian();

	float quotient = InvPi * 0.5f * radian;
	if (radian >= 0.f)
		quotient = static_cast<float>(static_cast<int>(quotient + 0.5f));
	else
		quotient = static_cast<float>(static_cast<int>(quotient - 0.5f));
	float y = radian - TwoPi *quotient;

	float sign;
	if (y > HalfPi)
	{
		y = Pi - y;
		sign = -1.f;
	}
	else if (y < -HalfPi)
	{
		y = -Pi - y;
		sign = -1.f;
	}
	else
		sign = 1.f;

	const f32 y2 = y * y;

	aSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

	const f32 p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
	aCos = sign * p;
}
