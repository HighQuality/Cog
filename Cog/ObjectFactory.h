#pragma once
#include "Factory.h"
#include "ObjectFactoryChunk.h"
#include "BaseObjectFactory.h"

template <typename T>
class ObjectFactory final : public Factory<T, ObjectFactoryChunk<T>>, public BaseObjectFactory
{
public:
	using Base = Factory<T, ObjectFactoryChunk<T>>;

	ObjectFactory(const TypeID<Object> aWidgetTypeID)
		: BaseObjectFactory(aWidgetTypeID)
	{
	}

	void IterateChunks(FunctionView<void(BaseObjectFactoryChunk&)> aCallback) override
	{
		for (ObjectFactoryChunk<T>* chunk : this->myChunks)
			aCallback(*chunk);
	}

protected:
	Object& AllocateGeneric() override
	{
		return this->Allocate();
	}

	void ReturnGeneric(const Object& aComponent) override
	{
		this->Return(CastChecked<T>(aComponent));
	}
};
