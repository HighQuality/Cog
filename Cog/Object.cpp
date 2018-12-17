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

void Object::Destroy()
{
	CHECK(IsInGameThread());
	Destroyed();
	myChunk->ReturnByID(myChunkIndex);
}

void Object::Destroyed()
{
}
