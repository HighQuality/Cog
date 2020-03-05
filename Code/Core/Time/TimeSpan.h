#pragma once

class TimeSpan
{
public:
	FORCEINLINE static TimeSpan FromTicks(const i64 aTicks)
	{
		TimeSpan timeSpan;
		timeSpan.myTicks = aTicks;
		return timeSpan;
	}

	FORCEINLINE static TimeSpan FromMicroseconds(const f32 aMicroseconds)
	{
		return FromTicks(static_cast<i64>(aMicroseconds * 10.f));
	}

	FORCEINLINE static TimeSpan FromMicroseconds(const i64 aMicroseconds)
	{
		return FromTicks(aMicroseconds * 10);
	}

	FORCEINLINE static TimeSpan FromMilliseconds(const f32 aMilliseconds)
	{
		return FromMicroseconds(aMilliseconds * 1000.f);
	}

	FORCEINLINE static TimeSpan FromMilliseconds(const i64 aMilliseconds)
	{
		return FromMicroseconds(aMilliseconds * 1000);
	}

	FORCEINLINE static TimeSpan FromSeconds(const f32 aSeconds)
	{
		return FromMilliseconds(aSeconds * 1000.f);
	}

	FORCEINLINE static TimeSpan FromSeconds(const i64 aSeconds)
	{
		return FromMilliseconds(aSeconds * 1000);
	}

	FORCEINLINE static TimeSpan FromMinutes(const f32 aMinutes)
	{
		return FromSeconds(aMinutes * 60.f);
	}

	FORCEINLINE static TimeSpan FromMinutes(const i64 aMinutes)
	{
		return FromSeconds(aMinutes * 60);
	}

	FORCEINLINE static TimeSpan FromHours(const f32 aHours)
	{
		return FromMinutes(aHours * 60.f);
	}

	FORCEINLINE static TimeSpan FromHours(const i64 aHours)
	{
		return FromMinutes(aHours * 60);
	}

	FORCEINLINE static TimeSpan FromDays(const f32 aDays)
	{
		return FromHours(aDays * 24.f);
	}

	FORCEINLINE static TimeSpan FromDays(const i64 aDays)
	{
		return FromHours(aDays * 24);
	}

	FORCEINLINE static TimeSpan FromWeeks(const f32 aWeeks)
	{
		return FromDays(aWeeks * 7.f);
	}

	FORCEINLINE static TimeSpan FromWeeks(const i64 aWeeks)
	{
		return FromDays(aWeeks * 7);
	}

	FORCEINLINE i64 GetTotalTicks() const
	{
		return myTicks;
	}

	FORCEINLINE f32 GetTotalMicroseconds() const
	{
		return static_cast<float>(GetTotalTicks()) * 0.1f;
	}

	FORCEINLINE f32 GetTotalMilliseconds() const
	{
		return GetTotalMicroseconds() * 0.001f;
	}

	FORCEINLINE f32 GetTotalSeconds() const
	{
		return GetTotalMilliseconds() * 0.001f;
	}

	FORCEINLINE f32 GetTotalMinutes() const
	{
		return GetTotalSeconds() * (1.f / 60.f);
	}

	FORCEINLINE f32 GetTotalHours() const
	{
		return GetTotalMinutes() * (1.f / 60.f);
	}

	FORCEINLINE f32 GetTotalDays() const
	{
		return GetTotalHours() * (1.f / 24.f);
	}

	FORCEINLINE f32 GetTotalWeeks() const
	{
		return GetTotalDays() * (1.f / 7.f);
	}

	FORCEINLINE bool operator>(TimeSpan aRight) const
	{
		return myTicks > aRight.myTicks;
	}

	FORCEINLINE bool operator>=(TimeSpan aRight) const
	{
		return myTicks >= aRight.myTicks;
	}

	FORCEINLINE bool operator<(TimeSpan aRight) const
	{
		return myTicks < aRight.myTicks;
	}

	FORCEINLINE bool operator<=(TimeSpan aRight) const
	{
		return myTicks <= aRight.myTicks;
	}
	
	FORCEINLINE bool operator==(TimeSpan aRight) const
	{
		return myTicks == aRight.myTicks;
	}

	FORCEINLINE bool operator!=(TimeSpan aRight) const
	{
		return myTicks != aRight.myTicks;
	}
	
	FORCEINLINE TimeSpan operator+(TimeSpan aRight) const
	{
		return FromTicks(myTicks + aRight.myTicks);
	}
	
	FORCEINLINE TimeSpan operator-(TimeSpan aRight) const
	{
		return FromTicks(myTicks - aRight.myTicks);
	}

private:
	i64 myTicks = 0;
};
