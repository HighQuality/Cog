#pragma once
#include "Resource.h"

class Image;

class ImageResource : public Resource
{
public:
	using Base = Resource;

	void BeginLoad(const StringView& aResourcePath) override;

private:
	String myFileName;

	void FileLoaded(const ArrayView<u8>& aFileData, BinaryData& aOutput);

	void ImageLoaded(Image aImage);

protected:
	i32 GetVersion() override { return -1; }
};
