#include "pch.h"
#include "ImageResource.h"

void ImageResource::BeginLoad(const StringView& aResourcePath)
{
	LoadFile(aResourcePath, ObjectFunctionView<BinaryData(const ArrayView<u8>&)>(*this, &ImageResource::FileLoaded));
}

BinaryData ImageResource::FileLoaded(const ArrayView<u8>& aFileData)
{
}
