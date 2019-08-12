#include "pch.h"
#include "Random.h"
#include <ctime>

thread_local pcg32 gRandom(time(nullptr));

f32 RandFloat(f32 aMinInclusive = 0.f, f32 aMaxExclusive = 1.f)
{
	return aMinInclusive + gRandom.nextFloat() * (aMaxExclusive - aMinInclusive);
}

i32 RandInteger(i32 aMinInclusive = 0, i32 aMaxExclusive = MaxOf<i32>)
{
	return aMinInclusive + static_cast<i32>(gRandom.nextUInt(aMaxExclusive - aMinInclusive));
}
