#pragma once
#include "FunctionView.h"
#include "TypeID.h"

class Component;
class BaseComponentFactoryChunk;

class BaseComponentFactory
{
protected:
	BaseComponentFactory(TypeID<Component> aTypeID)
	{
		myTypeID = aTypeID;
	}

public:
	virtual ~BaseComponentFactory() {  }

	virtual void IterateChunks(FunctionView<void(BaseComponentFactoryChunk&)> aCallback) = 0;

	virtual Component& AllocateGeneric() = 0;
	virtual void ReturnGeneric(const Component& aComponent) = 0;
	FORCEINLINE TypeID<Component> GetTypeID() const { return myTypeID; }

private:
	TypeID<Component> myTypeID;
};
