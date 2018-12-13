#pragma once
#include "Factory.h"
#include "BaseComponentFactory.h"
#include "ComponentFactoryChunk.h"

template <typename T>
class ComponentFactory final : public Factory<T, ComponentFactoryChunk<T>>, public BaseComponentFactory
{
public:
	using Base = Factory<T, ComponentFactoryChunk<T>>;

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
		return this->Allocate();
	}

	void ReturnGeneric(const Component& aComponent) override
	{
		CHECK(dynamic_cast<T*>(&aComponent));

		this->Return(aComponent);
	}
};
