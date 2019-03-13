#pragma once
#include "Object.h"
#include "BinaryData.h"
#include "ObjectFunctionView.h"

using ResourceLoaderFunction = ObjectFunctionView<void(const ArrayView<u8>&, BinaryData&)>;

class Resource : public Object
{
public:
	using Base = Object;

protected:
	Resource() = default;

	friend class ResourceManager;
	
	virtual void BeginLoad(const StringView& aResourcePath) = 0;

	// Return -1 in order to disable caching
	virtual i32 GetVersion() = 0;
	
	static Array<u8> ReadEntireFile(const StringView& aFile);

	void RegisterCallback(const Function<void(Resource&)>& aFunctionCallback);

private:
	bool myIsLoaded = false;
};

