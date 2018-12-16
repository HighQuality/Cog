#pragma once
#include "FunctionView.h"
#include "TypeID.h"

class Object;
class BaseObjectFactoryChunk;

class BaseObjectFactory
{
protected:
	BaseObjectFactory(TypeID<Object> aTypeID)
	{
		myTypeID = aTypeID;
	}

public:
	virtual ~BaseObjectFactory() = default;

	virtual void IterateChunks(FunctionView<void(BaseObjectFactoryChunk&)> aCallback) = 0;

	virtual Object& AllocateGeneric() = 0;
	virtual void ReturnGeneric(const Object& aComponent) = 0;
	FORCEINLINE TypeID<Object> GetTypeID() const { return myTypeID; }

private:
	TypeID<Object> myTypeID;
};
