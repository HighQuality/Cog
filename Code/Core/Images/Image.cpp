#include "CorePch.h"
#include "Image.h"
#include "LinearColor.h"
#include <External/lodepng.h>

Image::Image()
{
	myWidth = 0;
	myHeight = 0;
}

Image::Image(const i32 aWidth, const i32 aHeight)
{
	myPixels.Resize(aWidth * aHeight);
	myWidth = aWidth;
	myHeight = aHeight;
}

Image::Image(Array<ByteColor> aPixels, const i32 aWidth, const i32 aHeight)
{
	CHECK(aPixels.GetLength() == aWidth * aHeight);
	myPixels = Move(aPixels);
	myWidth = aWidth;
	myHeight = aHeight;
}

Image::Image(const StringView& aFileName)
{
	std::vector<unsigned char> data;

	const auto loadErrorCode = lodepng::load_file(data, aFileName.ToStdString());

	if (loadErrorCode != 0)
	{
		Println(L"Failed to load texture ", aFileName, L": ", lodepng_error_text(loadErrorCode));
		return;
	}

	std::vector<unsigned char> image;
	unsigned width, height;

	const auto decodeErrorCode = lodepng::decode(image, width, height, data);

	if (decodeErrorCode != 0)
	{
		Println(L"Failed to decode texture ", aFileName, L": ", lodepng_error_text(decodeErrorCode));
		return;
	}

	myWidth = CastBoundsChecked<i32>(width);
	myHeight = CastBoundsChecked<i32>(height);

	myPixels.Resize(myWidth * myHeight);
	for (i32 y = 0; y < myHeight; ++y)
	{
		const i32 rowStart = y * myWidth;
		const i32 rowStartBytes = rowStart * 4;

		for (i32 x = 0; x < myWidth; ++x)
		{
			ByteColor& color = myPixels[rowStart + x];
			const i32 colorStart = rowStartBytes + x * 4;
			color.r = image[colorStart + 0];
			color.g = image[colorStart + 1];
			color.b = image[colorStart + 2];
			color.a = image[colorStart + 3];
		}
	}
}


Image::~Image()
{
}

Array<LinearColor> Image::ToLinearColorArray() const
{
	Array<LinearColor> out;
	out.Resize(myPixels.GetLength());

	for (i32 i = 0; i < myPixels.GetLength(); ++i)
		out[i] = myPixels[i].ToLinear();

	return out;
}
