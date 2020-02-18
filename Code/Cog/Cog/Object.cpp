#include "CogPch.h"
#include "Object.h"
#include "CogGame.h"
#include "Program.h"
#include "CogTypeChunk.h"

bool Object::Destroy()
{
	if (IsPendingDestroy())
		return false;

	myChunk->MarkPendingDestroy(myChunkIndex);
	GetProgram().ScheduleDestruction(*this);
	
	return true;
}

ProgramContext& Object::GetProgramContext() const
{
	return GetChunk().GetProgramContext();
}

Ptr<Object> Object::NewChildByType(const TypeID<CogTypeBase>& aType)
{
	return GetProgram().NewObjectByType(aType, this);
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

const Ptr<Object>& Object::GetOwner() const
{
	return GetChunk().GetOwner(myChunkIndex);
}
