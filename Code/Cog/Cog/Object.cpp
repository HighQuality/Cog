#include "CogPch.h"
#include "Object.h"
#include "CogGame.h"
#include "CogTypeChunk.h"

Object::Object() = default;

Object::~Object() = default;

bool Object::Destroy()
{
	if (IsPendingDestroy())
		return false;

	myChunk->MarkPendingDestroy(myChunkIndex);

	GetGame().ScheduleDestruction(*this);
	
	return true;
}

void Object::Destroyed()
{
}

void Object::ReturnToAllocator()
{
	Destroyed();

	ObjectReturner(*myChunk, myChunkIndex);
}

bool Object::IsPendingDestroy() const
{
	return GetChunk().IsPendingDestroy(myChunkIndex);
}

u8 Object::GetGeneration() const
{
	return GetChunk().GetGeneration(myChunkIndex);
}
