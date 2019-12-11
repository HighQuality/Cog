#include "CogPch.h"
#include "Object.h"
#include <Memory/FactoryChunk.h>
#include "CogGame.h"

Object::Object() = default;

Object::~Object() = default;

bool Object::Destroy()
{
	if (IsPendingDestroy())
		return false;

	GetGame().ScheduleDestruction(*this);
	return true;
}

void Object::Destroyed()
{
}

void Object::ReturnToAllocator()
{
	Destroyed();

	myChunk->ReturnByIndex(myChunkIndex);
}
