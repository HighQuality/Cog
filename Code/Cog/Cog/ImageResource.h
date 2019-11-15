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

protected:
	i32 GetVersion() override { return -1; }
};
