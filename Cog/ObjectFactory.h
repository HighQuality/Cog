#pragma once
#include "Factory.h"
#include "ObjectFactoryChunk.h"
#include "BaseObjectFactory.h"

template <typename T>
class ObjectFactory final : public FactoryImplementation<T, ObjectFactoryChunk<T>>, public BaseObjectFactory
{
public:
	using Base = FactoryImplementation<T, ObjectFactoryChunk<T>>;

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
		return *static_cast<Object*>(this->AllocateRawObject());
	}

	void ReturnGeneric(const Object& aComponent) override
	{
		this->ReturnRawObject(static_cast<const void*>(&aComponent));
	}
};
