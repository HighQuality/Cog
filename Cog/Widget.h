#pragma once
#include "Event.h"

class BaseWidgetFactoryChunk;
class RenderTarget;

class Widget
{
public:
	Widget();
	~Widget();

	void Destroy();

	Event<Widget&> OnDestroyed;

protected:
	virtual void Tick(const Time& aDeltaTime);
	virtual void Draw(RenderTarget& aRenderTarget) const;

private:
	template <typename T>
	friend class Ptr;
	template <typename T>
	friend class WidgetFactoryChunk;

	BaseWidgetFactoryChunk* myChunk = nullptr;
	u16 myChunkIndex = 0;
};
