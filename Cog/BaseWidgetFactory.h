#pragma once
#include "FunctionView.h"
#include "TypeID.h"

class Widget;
class BaseWidgetFactoryChunk;

class BaseWidgetFactory
{
protected:
	BaseWidgetFactory(TypeID<Widget> aTypeID)
	{
		myTypeID = aTypeID;
	}

public:
	virtual ~BaseWidgetFactory() = default;

	virtual void IterateChunks(FunctionView<void(BaseWidgetFactoryChunk&)> aCallback) = 0;

	virtual Widget& AllocateGeneric() = 0;
	virtual void ReturnGeneric(const Widget& aComponent) = 0;
	FORCEINLINE TypeID<Widget> GetTypeID() const { return myTypeID; }

private:
	TypeID<Widget> myTypeID;
};
