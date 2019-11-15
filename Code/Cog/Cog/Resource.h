#pragma once
#include <Containers/BinaryData.h>
#include "Object.h"
#include "ObjectFunctionView.h"
#include "Resource.generated.h"

using ResourceLoaderFunction = ObjectFunctionView<void(const ArrayView<u8>&, BinaryData&)>;

COGTYPE()
class Resource : public Object
{
	GENERATED_BODY;
public:

protected:
	Resource() = default;

	friend class ResourceManager;
	
	virtual void BeginLoad(const StringView& aResourcePath) = 0;

	// Return -1 in order to disable caching
	virtual i32 GetVersion() = 0;

	//GENCELFIXED
	void RegisterCallback(const Function<void(Resource&)>& aFunctionCallback);

private:
	bool myIsLoaded = false;
};
