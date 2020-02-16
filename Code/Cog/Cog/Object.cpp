#include "CogPch.h"
#include "Object.h"
#include "CogGame.h"
#include "Program.h"
#include "CogTypeChunk.h"

Object::Object() = default;

Object::~Object() = default;

bool Object::Destroy()
{
	if (IsPendingDestroy())
		return false;

	myChunk->MarkPendingDestroy(myChunkIndex);

	if (&GetProgram() != this)
		GetProgram().ScheduleDestruction(*this);
	
	return true;
}

Ptr<Object> Object::NewChildByType(const TypeID<Object>& aType)
{
	return GetProgram().NewObjectByType(aType, *this);
}

void Object::Created()
{
	myBaseCalled = 1;
}

void Object::Destroyed()
{
	myBaseCalled = 1;
}

void Object::ReturnToAllocator()
{
	myBaseCalled = 0;
	Destroyed();
	CHECK_MSG(myBaseCalled == 1, L"Object subclass did not call Base::Destroyed() all the way down to Object");

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

Program& Object::GetProgram() const
{
	return GetChunk().GetProgram();
}

void Object::SetOwner(const Ptr<Object>& aNewObject)
{
	GetChunk().SetOwner(myChunkIndex, aNewObject);
}

const Ptr<Object>& Object::GetOwner() const
{
	return GetChunk().GetOwner(myChunkIndex);
}
