#pragma once

#include "CorePch.h"

#define COGTYPE(...)
#define COGFUNC(...)

#define COGLISTENER

// Should be placed very first on all class or struct COGTYPEs followed by a semicolon.
#define GENERATED_BODY JOIN(JOIN(GENERATED_BODY, COG_CURRENT_GENERATED_HEADER_FILE), __LINE__)

bool IsInGameThread();

#define CHECK_COMPONENT_INCLUDED(Type) do { static_assert(IsComplete<Type>, "Component header must be included"); } while (false)

#include <Cog/Casts.h>
#include <Cog/Class.h>
#include <Cog/Pointer.h>
#include <Cog/FrameData.h>

class TypeList;

void RunGame();

Object& NewObjectByType(const TypeID<Object>& aTypeID);

template <typename T>
T& NewObject(const Class<T>& aType = Class<T>())
{
	return (T&)NewObjectByType(aType);
}
