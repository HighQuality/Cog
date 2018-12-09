#include "pch.h"
#include "DrawCall.h"
#include "Texture.h"
#include "VertexBuffer.h"

thread_local DrawCallList DrawCall::ourLocalDrawCallList;

DrawCall::DrawCall() = default;
DrawCall::~DrawCall() = default;

DrawCall::DrawCall(DrawCall&&) = default;
DrawCall::DrawCall(const DrawCall&) = default;
DrawCall& DrawCall::operator=(DrawCall&&) = default;
DrawCall& DrawCall::operator=(const DrawCall&) = default;

void DrawCall::Submit()
{
	ourLocalDrawCallList.Add(Move(*this));
}

Array<DrawCall> DrawCallList::GatherDrawCalls()
{
	std::lock_guard<std::mutex> lck(ourInstancesMutex);

	Array<DrawCall> drawCalls;

	i32 count = 0;

	for (DrawCallList* list : ourInstances)
		count += list->GetLength();
	
	drawCalls.PrepareAdd(count);

	for (DrawCallList* list : ourInstances)
	{
		for (DrawCall& drawCall : *list)
			drawCalls.Add(Move(drawCall));
		list->Clear();
	}

	return drawCalls;
}

Array<DrawCallList*> DrawCallList::ourInstances;
std::mutex DrawCallList::ourInstancesMutex;
