#include "pch.h"
#include "Random.h"
#include <UtilitiesTLS.h>
#include <External/pcg32.h>

f32 RandFloat(const f32 aMinInclusive, const f32 aMaxExclusive)
{
	return aMinInclusive + UtilitiesTLS::GetRandomEngine().nextFloat() * (aMaxExclusive - aMinInclusive);
}

i32 RandInteger(const i32 aMinInclusive, const i32 aMaxExclusive)
{
	return aMinInclusive + static_cast<i32>(UtilitiesTLS::GetRandomEngine().nextUInt(aMaxExclusive - aMinInclusive));
}
