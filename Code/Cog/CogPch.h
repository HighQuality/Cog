#pragma once

#include "CorePch.h"

#define COGTYPE(...)
#define COGFUNC(...)
#define COGPROPERTY(Declaration, ...)

#define COGLISTENER

// Should be placed very first on all class or struct COGTYPEs followed by a semicolon.
#define GENERATED_BODY JOIN(JOIN(GENERATED_BODY, COG_CURRENT_GENERATED_HEADER_FILE), __LINE__)

bool IsInGameThread();

#include <Memory/ManualInitializationObject.h>

#include <Cog/Casts.h>
#include <Cog/Class.h>
#include <Cog/Pointer.h>
#include <Cog/FrameData.h>

class TypeList;

void RunGame();

Ptr<Object> NewObjectByType(const TypeID<Object>& aTypeID);

template <typename T>
Ptr<T> NewObject(const Class<T>& aType = Class<T>())
{
	return NewObjectByType(aType).Get();
}
