#pragma once

f32 RandomFloat(f32 aMinInclusive = 0.f, f32 aMaxExclusive = 1.f);

i32 RandomInteger(i32 aMinInclusive = 0, i32 aMaxExclusive = MaxOf<i32>);

i32 RandomIndex(i32 aLength);
