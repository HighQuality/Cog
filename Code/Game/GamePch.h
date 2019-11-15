#pragma once

#include "CogPch.h"

/**
	Should be used in attribute specifier sequence, e.g:
	[[COGTYPE()]] class X : public Object { ... };
	[[COGFUNC()]] void Function();

	https://en.cppreference.com/w/cpp/language/attributes
	"All attributes unknown to an implementation are ignored without causing an error. (since C++17)"
*/
#define COGTYPE(...)
#define COGFUNC(...)

// Should be placed very first on all class or struct COGTYPEs followed by a semicolon.
#define GENERATED_BODY JOIN(JOIN(GENERATED_BODY, COG_CURRENT_GENERATED_HEADER_FILE), __LINE__)

using ChunkIndex = u8;

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
