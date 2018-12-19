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

	template <typename TReturn, typename TType>
	void DoLoadAction(TType* aThisPtr, Function<TReturn()> aWork, void(TType::*aFunction)(TReturn))
	{
		CHECK(aThisPtr == this);
		ResourceManager& resourceManager = GetResourceManager();
		ObjectFunctionView<void(TReturn)> func(*aThisPtr, aFunction);
		resourceManager.DoLoadAction(Move(aWork), func);
	}

	static Array<u8> ReadEntireFile(const StringView& aFile);

private:
	bool myIsLoaded = false;
};

