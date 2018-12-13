#pragma once
#include "FactoryChunk.h"
#include "BaseComponentFactoryChunk.h"

template <typename T>
class ComponentFactoryChunk final : public FactoryChunk<T>, public BaseComponentFactoryChunk
{
public:
	using Base = FactoryChunk<T>;

	ComponentFactoryChunk(const u16 aSize)
		: Base(aSize), BaseComponentFactoryChunk(aSize)
	{
	}

	T& Allocate() override
	{
		T& object = Base::Allocate();
		object.myChunk = this;
		object.myChunkIndex = this->IndexOf(object);

		InitializeSOAProperties(object.myChunkIndex);

		return object;
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
		if (typeid(&Component::Tick) == typeid(&T::Tick))
			return;

		this->IteratePotentialIndices([this, aDeltaTime](const u16 aIndex)
		{
			if (IsTickEnabled(aIndex))
				this->myObjectsData[aIndex].T::Tick(aDeltaTime);
		});
	}

	void DispatchDraw2D(RenderTarget& aRenderTarget) override
	{
		// Skip classes that does not override the function
		if (typeid(&Component::Draw2D) == typeid(&T::Draw2D))
			return;

		this->IteratePotentialIndices([this, &aRenderTarget](const u16 aIndex)
		{
			if (IsVisible(aIndex))
				this->myObjectsData[aIndex].T::Draw2D(aRenderTarget);
		});
	}

	void DispatchDraw3D(RenderTarget& aRenderTarget) override
	{
		// Skip classes that does not override the function
		if (typeid(&Component::Draw3D) == typeid(&T::Draw3D))
			return;

		this->IteratePotentialIndices([this, &aRenderTarget](const u16 aIndex)
		{
			if (IsVisible(aIndex))
				this->myObjectsData[aIndex].T::Draw3D(aRenderTarget);
		});
	}
};
