#include "pch.h"
#include "Component.h"

void Component::SetTickEnabled(const bool aShouldTick)
{
	myChunk->SetTickEnabled(myChunkIndex, aShouldTick);
}

void Component::SetIsVisible(const bool aIsVisible)
{
	myChunk->SetIsVisible(myChunkIndex, aIsVisible);
}
