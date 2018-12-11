#include "pch.h"
#include "Component.h"
#include "BaseComponentFactoryChunk.h"

Component::Component()
{
}

Component::~Component()
{
}

bool Component::ShouldTick() const
{
	return myChunk->IsTickEnabled(*this);
}

bool Component::IsVisible() const
{
	return myChunk->IsVisible(*this);
}

void Component::SetShouldTick(const bool aShouldTick)
{
	myChunk->SetTickEnabled(*this, aShouldTick);
}

void Component::SetIsVisible(bool aIsVisible)
{
	myChunk->SetIsVisible(*this, aIsVisible);
}

void Component::ResolveDependencies()
{
}
