#pragma once
#include "Event.h"

class BaseObjectFactoryChunk;
class RenderTarget;

class Object
{
public:
	using Base = void;

	Object();
	virtual ~Object();

	void Destroy();

protected:
	virtual void Destroyed();

private:
	template <typename T>
	friend class Ptr;
	template <typename T>
	friend class ObjectFactoryChunk;

	BaseObjectFactoryChunk* myChunk = nullptr;
	u16 myChunkIndex = 0;
};
