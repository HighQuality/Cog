#include "pch.h"
#include "ByteColor.h"
#include "LinearColor.h"

LinearColor ByteColor::ToLinear() const
{
	return LinearColor(static_cast<f32>(r) / 255.f,
		static_cast<f32>(g) / 255.f,
		static_cast<f32>(b) / 255.f,
	    static_cast<f32>(a) / 255.f);
}
