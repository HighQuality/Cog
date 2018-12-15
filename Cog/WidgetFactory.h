#pragma once
#include "Factory.h"
#include "WidgetFactoryChunk.h"
#include "BaseWidgetFactory.h"

template <typename T>
class WidgetFactory final : public Factory<T, WidgetFactoryChunk<T>>, public BaseWidgetFactory
{
public:
	using Base = Factory<T, WidgetFactoryChunk<T>>;

	WidgetFactory(const TypeID<Widget> aWidgetTypeID)
		: BaseWidgetFactory(aWidgetTypeID)
	{
	}

	void IterateChunks(FunctionView<void(BaseWidgetFactoryChunk&)> aCallback) override
	{
		for (WidgetFactoryChunk<T>* chunk : this->myChunks)
			aCallback(*chunk);
	}

protected:
	Widget& AllocateGeneric() override
	{
		return this->Allocate();
	}

	void ReturnGeneric(const Widget& aComponent) override
	{
		this->Return(CastChecked<T>(aComponent));
	}
};
