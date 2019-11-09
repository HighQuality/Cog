#pragma once

#include "CogPch.h"

#define COGTYPE(...)

using ChunkIndex = u8;

bool IsInGameThread();

#define CHECK_COMPONENT_INCLUDED(Type) do { static_assert(IsComplete<Type>, "Component header must be included"); } while (false)

#include <Cog/InheritType.h>

#include <Cog/Casts.h>
#include <Cog/Class.h>
#include <Cog/Pointer.h>
#include <Cog/FrameData.h>

class TypeList;

void InnerRunGame(UniquePtr<TypeList>(*aTypeListCreator)());

template <typename TTypeList>
void RunGame()
{
	InnerRunGame([]() -> UniquePtr<TypeList> { return MakeUnique<TTypeList>(); });
}

Object& NewObjectByType(const TypeID<Object>& aTypeID);

template <typename T>
T& NewObject(const Class<T>& aType = Class<T>())
{
	return (T&)NewObjectByType(aType);
}
