#pragma once

class Component;
class RenderTarget;

class BaseComponentFactoryChunk
{
public:
	BaseComponentFactoryChunk();
	virtual ~BaseComponentFactoryChunk();

	virtual void DispatchTick(Time aDeltaTime) = 0;
	virtual void DispatchDraw2D(RenderTarget& aRenderTarget) = 0;
	virtual void DispatchDraw3D(RenderTarget& aRenderTarget) = 0;

	virtual u16 FindGeneration(Component& aComponent) = 0;

protected:
	friend Component;

	virtual void SetTickEnabled(const Component& aComponent, bool aTickEnabled) = 0;
	virtual bool IsTickEnabled(const Component& aComponent) const = 0;

	virtual void SetIsVisible(const Component& aComponent, bool aIsVisible) = 0;
	virtual bool IsVisible(const Component& aComponent) const = 0;
};

