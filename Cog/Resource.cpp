#include "pch.h"
#include "Resource.h"
#include "ResourceManager.h"

void Resource::LoadFile(const StringView& aFileName, ObjectFunctionView<BinaryData(const ArrayView<u8>&)> aFileLoadedCallback)
{
	GetResourceManager().LoadFile(aFileName, aFileLoadedCallback);
}
