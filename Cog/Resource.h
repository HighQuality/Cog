#pragma once
#include "Object.h"
#include "BinaryData.h"

class Resource : public Object
{
public:

protected:
	friend class ResourceManager;
	virtual void BeginLoad(const StringView& aResourcePath) { FATAL_PURE_VIRTUAL(); }

	void LoadFile(const StringView& aFileName, ObjectFunctionView<BinaryData(const ArrayView<u8>&)> aFileLoadedCallback);

private:
	bool myIsLoaded = false;
};

