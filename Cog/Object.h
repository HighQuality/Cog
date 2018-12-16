#pragma once
#include "Event.h"

class BaseObjectFactoryChunk;
class RenderTarget;

class Object
{
public:
	using Base = void;

	Object();
	~Object();

	void Destroy();

	Event<Object&> OnDestroyed;

protected:
	virtual void Tick(const Time& aDeltaTime);
	virtual void Draw(RenderTarget& aRenderTarget) const;

private:
	template <typename T>
	friend class Ptr;
	template <typename T>
	friend class ObjectFactoryChunk;

	BaseObjectFactoryChunk* myChunk = nullptr;
	u16 myChunkIndex = 0;
};
