#include "pch.h"
#include "Component.h"

Component::~Component()
{
	myChunk = nullptr;
	myChunkIndex = 0;
}

void Component::SetTickEnabled(const bool aShouldTick)
{
	myChunk->SetTickEnabled(myChunkIndex, aShouldTick);
}

void Component::SetIsVisible(const bool aIsVisible)
{
	myChunk->SetIsVisible(myChunkIndex, aIsVisible);
}
