#pragma once

struct Angle
{
	FORCEINLINE Angle()
	{
		myRadian = 0.f;
	}

	// Construct using Radian(x) or Degrees(x)
	FORCEINLINE Angle(f32 aRadianAngle, decltype(nullptr))
	{
		myRadian = aRadianAngle;
	}

	FORCEINLINE ~Angle() = default;

	FORCEINLINE Angle(const Angle&) = default;

	FORCEINLINE f32 AsRadian() const
	{
		return myRadian;
	}
	
	FORCEINLINE f32 AsDegrees() const
	{
		return myRadian / Pi * 180.f;
	}

private:
	f32 myRadian;
};

FORCEINLINE Angle Radian(const f32 aRadianRotation)
{
	return Angle(aRadianRotation, nullptr);
}

FORCEINLINE Angle Degrees(const f32 aDegreeRotation)
{
	return Angle(aDegreeRotation / 180.f * Pi, nullptr);
}
