#pragma once
#include <Containers/BinaryData.h>
#include "TickEvent.h"
#include "Resource.generated.h"

COGTYPE()
class Resource : public Object
{
	GENERATED_BODY;

protected:
	virtual void BeginLoad(const StringView& aResourcePath) {}

	// Return -1 in order to disable caching
	virtual i32 GetVersion() { return 0; }

	COGLISTENER;
	void Tick(const TickEvent& aEvent);

	COGLISTENER;
	void FileLoaded(const FileLoadedImpulse& aImpulse);

private:
	COGPROPERTY(bool IsLoaded);
};
