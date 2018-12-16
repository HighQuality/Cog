#pragma once
#include "FactoryChunk.h"
#include "BaseObjectFactoryChunk.h"

template <typename T>
class ObjectFactoryChunk final : public FactoryChunk<T>, public BaseObjectFactoryChunk
{
public:
	using Base = FactoryChunk<T>;

	ObjectFactoryChunk(const u16 aSize)
		: Base(aSize), BaseObjectFactoryChunk(aSize)
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
		if (typeid(&Object::Tick) == typeid(&T::Tick))
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
		if (typeid(&Object::Draw) == typeid(&T::Draw))
			return;

		this->IteratePotentialIndices([this, &aRenderTarget](const u16 aIndex)
		{
			if (IsVisible(aIndex))
				this->myObjectsData[aIndex].T::Draw(aRenderTarget);
		});
	}
};
