#pragma once

class Time
{
public:
	Time()
	{
		mySeconds = 0.f;
	}

	static Time Milliseconds(const f32 aMilliseconds)
	{
		return Time(aMilliseconds / 1000.f);
	}

	static Time Seconds(const f32 aSeconds)
	{
		return Time(aSeconds);
	}

	static Time Minutes(const f32 aMinutes)
	{
		return Time(aMinutes * 60.f);
	}

	static Time Hours(const f32 aHours)
	{
		return Time(aHours * 60.f * 60.f);
	}

	static Time Days(const f32 aHours)
	{
		return Time(aHours * 60.f * 60.f * 24.f);
	}

	static Time Weeks(const f32 aHours)
	{
		return Time(aHours * 60.f * 60.f * 24.f * 7.f);
	}

	f32 Seconds() const
	{
		return mySeconds;
	}

	f32 Milliseconds() const
	{
		return mySeconds * 1000.f;
	}

	f32 Minutes() const
	{
		return mySeconds / 60.f;
	}

	f32 Hours() const
	{
		return mySeconds / 60.f / 60.f;
	}

	f32 Days() const
	{
		return mySeconds / 60.f / 60.f / 24.f;
	}

	f32 Weeks() const
	{
		return mySeconds / 60.f / 60.f / 24.f / 7.f;
	}

	FORCEINLINE bool operator>(Time aRight) const
	{
		return mySeconds > aRight.mySeconds;
	}

	FORCEINLINE bool operator>=(Time aRight) const
	{
		return mySeconds >= aRight.mySeconds;
	}

	FORCEINLINE bool operator<(Time aRight) const
	{
		return mySeconds < aRight.mySeconds;
	}

	FORCEINLINE bool operator<=(Time aRight) const
	{
		return mySeconds <= aRight.mySeconds;
	}
	
	FORCEINLINE bool operator==(Time aRight) const
	{
		return mySeconds == aRight.mySeconds;
	}

	FORCEINLINE bool operator!=(Time aRight) const
	{
		return mySeconds != aRight.mySeconds;
	}
	
	FORCEINLINE Time operator+(Time aRight) const
	{
		return Seconds(mySeconds + aRight.mySeconds);
	}
	
	FORCEINLINE Time operator-(Time aRight) const
	{
		return Seconds(mySeconds - aRight.mySeconds);
	}

private:
	explicit Time(const f32 aSeconds)
	{
		mySeconds = aSeconds;
	}

	f32 mySeconds;
};
