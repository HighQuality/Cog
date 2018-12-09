#pragma once
#include "ByteColor.h"

class Image
{
public:
	Image();
	Image(const StringView& aFileName);
	~Image();

	void SetPixel(const i32 aX, const i32 aY, const ByteColor& aColor) { CHECK(aX >= 0 && aY >= 0 && aX < myWidth && aY < myHeight); myPixels[aY * myWidth + aX] = aColor; }
	const ByteColor& GetPixel(const i32 aX, const i32 aY) const { CHECK(aX >= 0 && aY >= 0 && aX < myWidth && aY < myHeight); return myPixels[aY * myWidth + aX]; }

	Array<LinearColor> ToLinearColorArray() const;

	bool IsEmpty() const { return myWidth == 0 || myHeight == 0; }

	FORCEINLINE i32 GetWidth() const { return myWidth; }
	FORCEINLINE i32 GetHeight() const { return myHeight; }

private:
	Array<ByteColor> myPixels;
	i32 myWidth;
	i32 myHeight;
};

