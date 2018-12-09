#pragma once

struct LinearColor
{
	f32 r;
	f32 g;
	f32 b;
	f32 a;

	LinearColor()
	{
		r = 0.f;
		g = 0.f;
		b = 0.f;
		a = 0.f;
	}

	LinearColor(f32 r, f32 g, f32 b, f32 a = 1.f)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	const f32* GetData() const { return &r; }

	static LinearColor CornflowerBlue;
	static LinearColor White;
	static LinearColor Black;
	static LinearColor TransparentWhite;
	static LinearColor TransparentBlack;
};
