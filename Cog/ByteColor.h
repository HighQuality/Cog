#pragma once

struct ByteColor
{
	u8 r;
	u8 g;
	u8 b;
	u8 a;

	ByteColor(u8 r, u8 g, u8 b, u8 a = 255)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	ByteColor()
		: ByteColor(0, 0, 0, 0)
	{
	}

	LinearColor ToLinear() const;
};
