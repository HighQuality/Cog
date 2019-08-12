#pragma once
struct pcg32;

extern thread_local pcg32 gRandom;

f32 RandFloat(f32 aMinInclusive = 0.f, f32 aMaxExclusive = 1.f);

i32 RandInteger(i32 aMinInclusive = 0, i32 aMaxExclusive = MaxOf<i32>);
