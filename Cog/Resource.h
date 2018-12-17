#pragma once
#include "Object.h"

class Resource : public Object
{
public:

protected:
	virtual void BeginLoad();
	void RegisterCallback(ObjectFunctionView);

private:
	bool myIsLoaded = false;
};

