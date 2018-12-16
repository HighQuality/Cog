#include "pch.h"
#include "Object.h"

Object::Object()
{
}

Object::~Object()
{
	myChunk = nullptr;
	myChunkIndex = 0;
}

void Object::Tick(const Time& aDeltaTime)
{
}

void Object::Draw(RenderTarget& aRenderTarget) const
{
}

void Object::Destroy()
{
	OnDestroyed.Broadcast(*this);
	myChunk->ReturnByID(myChunkIndex);
}
