#pragma once
#include "Component.h"

class RenderTarget;

class BaseComponentFactoryChunk
{
public:
	BaseComponentFactoryChunk(const u16 aSize)
	{
		myGeneration.Resize(aSize);

		for (u16& generation : myGeneration)
			generation = 1;
	}

	virtual ~BaseComponentFactoryChunk() = default;

	virtual void DispatchTick(Time aDeltaTime) = 0;
	virtual void DispatchDraw2D(RenderTarget& aRenderTarget) = 0;
	virtual void DispatchDraw3D(RenderTarget& aRenderTarget) = 0;

	FORCEINLINE u16 FindGeneration(const Component& aComponent) const
	{
		return myGeneration[static_cast<i32>(&aComponent - myBasePointer)];
	}
	
protected:
	friend Component;

	virtual void SetTickEnabled(const Component& aComponent, bool aTickEnabled) = 0;
	virtual bool IsTickEnabled(const Component& aComponent) const = 0;

	virtual void SetIsVisible(const Component& aComponent, bool aIsVisible) = 0;
	virtual bool IsVisible(const Component& aComponent) const = 0;

	void SetBasePointer(const Component* aBasePointer)
	{
		myBasePointer = aBasePointer;
	}

	FORCEINLINE void IncrementGeneration(u16 aIndex)
	{
		myGeneration[aIndex]++;
	}

private:
	const Component* myBasePointer;
	Array<u16> myGeneration;
};
