#pragma once
#include "CogMath.h"
#include "Angle.h"

struct Vec2
{
	static const Vec2 Zero;
	static const Vec2 One;
	static const Vec2 Right;
	static const Vec2 Left;
	static const Vec2 Up;
	static const Vec2 Down;

	f32 x;
	f32 y;

	Vec2()
	{
		x = 0.f;
		y = 0.f;
	}

	~Vec2() = default;

	Vec2(const Vec2&) = default;

	Vec2(const float aX, const float aY)
	{
		x = aX;
		y = aY;
	}
	
	Vec2 operator+(const Vec2& aOther) const
	{
		return Vec2(x + aOther.x, y + aOther.y);
	}

	Vec2& operator+=(const Vec2& aOther)
	{
		x += aOther.x;
		y += aOther.y;
		return *this;
	}

	Vec2 operator-(const Vec2& aOther) const
	{
		return Vec2(x - aOther.x, y - aOther.y);
	}

	Vec2& operator-=(const Vec2& aOther)
	{
		x -= aOther.x;
		y -= aOther.y;
		return *this;
	}

	Vec2 operator-() const
	{
		return Vec2(-x, -y);
	}

	Vec2 operator*(const Vec2& aOther) const
	{
		return Vec2(x * aOther.x, y * aOther.y);
	}

	Vec2& operator*=(const Vec2& aOther)
	{
		x *= aOther.x;
		y *= aOther.y;
		return *this;
	}

	Vec2 operator/(const Vec2& aOther) const
	{
		return Vec2(x / aOther.x, y / aOther.y);
	}

	Vec2& operator/=(const Vec2& aOther)
	{
		x /= aOther.x;
		y /= aOther.y;
		return *this;
	}

	Vec2 operator*(const f32 aScalar) const
	{
		return Vec2(x * aScalar, y * aScalar);
	}

	Vec2& operator*=(const f32 aScalar)
	{
		x *= aScalar;
		y *= aScalar;
		return *this;
	}

	Vec2 operator/(const f32 aScalar) const
	{
		return Vec2(x / aScalar, y / aScalar);
	}
	
	Vec2& operator/=(const f32 aScalar)
	{
		x /= aScalar;
		y /= aScalar;
		return *this;
	}

	bool operator==(const Vec2& aOther) const
	{
		return x == aOther.x && y == aOther.y;
	}

	bool operator!=(const Vec2& aOther) const
	{
		return x != aOther.x ||
			y != aOther.y;
	}

	bool operator>(const Vec2& aOther) const
	{
		return x > aOther.x &&
			y > aOther.y;
	}

	bool operator<(const Vec2& aOther) const
	{
		return x < aOther.x &&
			y < aOther.y;
	}

	bool operator>=(const Vec2& aOther) const
	{
		return x >= aOther.x &&
			y >= aOther.y;
	}

	bool operator<=(const Vec2& aOther) const
	{
		return x <= aOther.x &&
			y <= aOther.y;
	}

	Vec2 GetRotated(const Angle aAngle) const
	{
		float sin, cos;
		SinCos(aAngle, sin, cos);

		return Vec2(
			cos * x - sin * y,
			sin *x + cos * y
		);
	}

	// Towards positive Y
	Vec2 GetRotated90Clockwise() const
	{
		return Vec2(
			-y,
			x
		);
	}

	// Towards negative Y
	Vec2 GetRotated90Counterclockwise() const
	{
		return Vec2(
			y,
			-x
		);
	}

	Vec2 GetNormalized() const
	{
		return *this / GetLength();
	}

	Vec2 GetSafeNormalized(const Vec2& aDefault = Vec2::Right) const
	{
		const f32 length = GetLength();
		if (length > 0.f)
			return *this / length;
		return aDefault;
	}

	f32 GetLengthSquared() const
	{
		return x * x + y * y;
	}

	f32 GetLength() const
	{
		return Sqrt(GetLengthSquared());
	}

	f32 GetDot(const Vec2& aSecond) const
	{
		return x * aSecond.x + y * aSecond.y;
	}
	
	static f32 GetDot(const Vec2& aFirst, const Vec2& aSecond)
	{
		return aFirst.GetDot(aSecond);
	}

	Vec2 GetDeltaTo(const Vec2& aSecond) const
	{
		return aSecond - *this;
	}

	static Vec2 GetDeltaTo(const Vec2& aFirst, const Vec2& aSecond)
	{
		return aFirst.GetDeltaTo(aSecond);
	}

	f32 GetDistanceSquaredTo(const Vec2& aSecond) const
	{
		return GetDeltaTo(aSecond).GetLengthSquared();
	}

	static f32 GetDistanceSquaredTo(const Vec2& aFirst, const Vec2& aSecond)
	{
		return aFirst.GetDistanceSquaredTo(aSecond);
	}

	f32 GetDistanceTo(const Vec2& aSecond) const
	{
		return Sqrt(GetDistanceSquaredTo(aSecond));
	}

	static f32 GetDistanceTo(const Vec2& aFirst, const Vec2& aSecond)
	{
		return aFirst.GetDistanceTo(aSecond);
	}
};
