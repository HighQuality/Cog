#pragma once
#include "FactoryChunk.h"
#include "BaseComponentFactoryChunk.h"

template <typename T>
class ComponentFactoryChunk final : public FactoryChunk<T>, public BaseComponentFactoryChunk
{
public:
	using Base = FactoryChunk<T>;

	ComponentFactoryChunk(const u16 aSize)
		: Base(aSize)
	{
		myGeneration.Resize(aSize);
		myReceiveTicks.Resize(aSize);
		myIsVisible.Resize(aSize);

		for (u16& generation : myGeneration)
			generation = 1;

		for (bool& tick : myReceiveTicks)
			tick = false;

		for (bool& visible : myIsVisible)
			visible = false;
	}

	T& Allocate() override
	{
		T& object = Base::Allocate();
		object.myChunk = this;

		const u16 index = this->IndexOf(object);

		myGeneration[index]++;
		myReceiveTicks[index] = true;
		myIsVisible[index] = true;

		return object;
	}

	void Return(const T& aObject) override
	{
		const u16 index = this->IndexOf(aObject);

		Base::Return(aObject);

		myGeneration[index]++;
		myReceiveTicks[index] = false;
		myIsVisible[index] = false;
	}

	u16 FindGeneration(Component& aComponent) override
	{
		return myGeneration[this->IndexOf(aComponent)];
	}

	void DispatchTick(const Time aDeltaTime) override
	{
		// Skip classes that does not override the function
		if (typeid(&Component::Tick) == typeid(&T::Tick))
			return;

		this->IteratePotentialIndices([this, aDeltaTime](const u16 aIndex)
		{
			if (myReceiveTicks[aIndex])
				static_cast<T&>(this->myObjectData[aIndex])->T::Tick(aDeltaTime);
		});
	}

	void DispatchDraw2D(RenderTarget& aRenderTarget) override
	{
		// Skip classes that does not override the function
		if (typeid(&Component::Draw2D) == typeid(&T::Draw2D))
			return;

		this->IteratePotentialIndices([this, &aRenderTarget](const u16 aIndex)
		{
			if (myIsVisible[aIndex])
				static_cast<T&>(this->myObjectData[aIndex])->T::Draw2D(aRenderTarget);
		});
	}

	void DispatchDraw3D(RenderTarget& aRenderTarget) override
	{
		// Skip classes that does not override the function
		if (typeid(&Component::Draw3D) == typeid(&T::Draw3D))
			return;

		this->IteratePotentialIndices([this, &aRenderTarget](const u16 aIndex)
		{
			if (myIsVisible[aIndex])
				static_cast<T&>(this->myObjectData[aIndex])->T::Draw3D(aRenderTarget);
		});
	}

protected:
	void SetIsVisible(const Component& aComponent, const bool aIsVisible) override
	{
		myIsVisible[this->IndexOf(static_cast<const T&>(aComponent))] = aIsVisible;
	}

	bool IsVisible(const Component& aComponent) const override
	{
		return myIsVisible[this->IndexOf(static_cast<const T&>(aComponent))];
	}

	void SetTickEnabled(const Component& aComponent, const bool aTickEnabled) override
	{
		myReceiveTicks[this->IndexOf(static_cast<const T&>(aComponent))] = aTickEnabled;
	}

	bool IsTickEnabled(const Component& aComponent) const override
	{
		return myReceiveTicks[this->IndexOf(static_cast<const T&>(aComponent))];
	}

private:
	Array<u16> myGeneration;
	Array<bool> myReceiveTicks;
	Array<bool> myIsVisible;
};
