#include "CorePch.h"
#include "Random.h"
#include <External/pcg32.h>

f32 RandomFloat(const f32 aMinInclusive, const f32 aMaxExclusive)
{
	return aMinInclusive + CoreTLS::GetRandomEngine().nextFloat() * (aMaxExclusive - aMinInclusive);
}

i32 RandomInteger(const i32 aMinInclusive, const i32 aMaxExclusive)
{
	return aMinInclusive + static_cast<i32>(CoreTLS::GetRandomEngine().nextUInt(aMaxExclusive - aMinInclusive));
}

i32 RandomIndex(const i32 aLength)
{
	return RandomInteger(0, aLength);
}
