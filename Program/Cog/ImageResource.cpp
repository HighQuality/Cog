#include "pch.h"
#include "ImageResource.h"
#include <Utilities/External/lodepng.h>
#include <Utilities/Image.h>

void ImageResource::BeginLoad(const StringView& aResourcePath)
{
	myFileName = aResourcePath;

	const String file(myFileName);
	
	// DoLoadAction(this, Function<Image()>([file]() -> Image
	// {
	// 	return Image(file);
	// }), &ImageResource::ImageLoaded);
}

void ImageResource::FileLoaded(const ArrayView<u8>& aFileData, BinaryData& aOutput)
{
	std::vector<unsigned char> image;
	unsigned width, height;

	const auto decodeErrorCode = lodepng::decode(image, width, height, aFileData.GetData(), aFileData.GetLength());

	if (decodeErrorCode != 0)
	{
		Println(L"Failed to decode texture ", myFileName, L": ", lodepng_error_text(decodeErrorCode));
		return;
	}
	
	const i32 bytesPerPixel = 4;
	CHECK(width * height * bytesPerPixel == image.size());

	aOutput.Write<i32>(width);
	aOutput.Write<i32>(height);
	aOutput.Write<i32>(bytesPerPixel);
	aOutput.WriteRaw(image.data(), CastBoundsChecked<i32>(image.size()));
}
