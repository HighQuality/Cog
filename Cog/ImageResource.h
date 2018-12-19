#pragma once
#include "Resource.h"

class ImageResource : public Resource
{
public:
	void BeginLoad(const StringView& aResourcePath);

private:
	BinaryData FileLoaded(const ArrayView<u8>& aFileData);
};
