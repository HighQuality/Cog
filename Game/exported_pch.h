#pragma once

#include "../WindowsPlatform/exported_pch.h"

using ChunkIndex = u8;

bool IsInGameThread();

#define CHECK_COMPONENT_INCLUDED(Type) do { static_assert(IsComplete<Type>, "Component header must be included"); } while (false)

#include <Cog/InheritType.h>

#include <Cog/Casts.h>
#include <Cog/Pointer.h>
#include <Cog/FrameData.h>

Object& NewObjectByType(const TypeID<Object>& aTypeID);

template <typename T>
T& NewObject()
{
	return static_cast<T&>(NewObjectByType(TypeID<Object>::Resolve<T>()));
}
