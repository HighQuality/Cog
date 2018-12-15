#pragma once
#include "FactoryChunk.h"
#include "BaseWidgetFactoryChunk.h"

template <typename T>
class WidgetFactoryChunk final : public FactoryChunk<T>, public BaseWidgetFactoryChunk
{
public:
	using Base = FactoryChunk<T>;

	WidgetFactoryChunk(const u16 aSize)
		: Base(aSize), BaseWidgetFactoryChunk(aSize)
	{
	}

	T& Allocate() override
	{
		T& widget = Base::Allocate();
		widget.myChunk = this;
		widget.myChunkIndex = this->IndexOf(widget);

		InitializeSOAProperties(widget.myChunkIndex);

		return widget;
	}

	void Return(const T& aObject) override
	{
		const u16 index = this->IndexOf(aObject);

		Base::Return(aObject);

		DestroySOAProperties(index);
	}

	void ReturnByID(u16 aIndex) override
	{
		Return(this->myObjectsData[aIndex]);
	}

	void DispatchTick(const Time aDeltaTime) override
	{
		// Skip classes that does not override the function
		if (typeid(&Widget::Tick) == typeid(&T::Tick))
			return;

		this->IteratePotentialIndices([this, aDeltaTime](const u16 aIndex)
		{
			if (IsTickEnabled(aIndex))
				this->myObjectsData[aIndex].T::Tick(aDeltaTime);
		});
	}

	void DispatchDraw(RenderTarget& aRenderTarget) override
	{
		// Skip classes that does not override the function
		if (typeid(&Widget::Draw) == typeid(&T::Draw))
			return;

		this->IteratePotentialIndices([this, &aRenderTarget](const u16 aIndex)
		{
			if (IsVisible(aIndex))
				this->myObjectsData[aIndex].T::Draw(aRenderTarget);
		});
	}
};
