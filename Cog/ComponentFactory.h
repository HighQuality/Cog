#pragma once
#include "Factory.h"
#include "BaseComponentFactory.h"
#include "ComponentFactoryChunk.h"

template <typename T>
class ComponentFactory final : public FactoryImplementation<T, ComponentFactoryChunk<T>>, public BaseComponentFactory
{
public:
	using Base = FactoryImplementation<T, ComponentFactoryChunk<T>>;

	ComponentFactory(TypeID<Component> componentTypeID)
		: BaseComponentFactory(componentTypeID)
	{
	}

	void IterateChunks(FunctionView<void(BaseComponentFactoryChunk&)> aCallback) override
	{
		for (ComponentFactoryChunk<T>* chunk : this->myChunks)
			aCallback(*chunk);
	}
	
protected:
	Component& AllocateGeneric() override
	{
		return *static_cast<Component*>(this->AllocateRawObject());
	}

	void ReturnGeneric(const Component& aComponent) override
	{
		this->ReturnRawObject(static_cast<const void*>(&aComponent));
	}
};
