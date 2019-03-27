#include "pch.h"
#include "Object.h"
#include <FactoryChunk.h>

Object::Object() = default;

Object::~Object() = default;

void Object::Destroy()
{
	if (IsPendingDestroy())
		return;

	GetGame().ScheduleDestruction(*this);
}

void Object::Destroyed()
{
}

void Object::SetActivated(bool aIsActivated)
{
	myChunk->SetActivated(myChunkIndex, aIsActivated);
}

void Object::ReturnToAllocator()
{
	Destroyed();

	myChunk->ReturnByIndex(myChunkIndex);
}
