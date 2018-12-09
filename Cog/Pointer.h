﻿#pragma once
#include "Object.h"
#include "Component.h"
#include "GameWorld.h"
#include "ObjectFactory.h"

template <typename T>
class Ptr
{
public:
	Ptr()
	{
		myPointer = nullptr;
		myGeneration = 0;
	}

	Ptr(T* aPointer)
		: Ptr()
	{
		if (aPointer)
		{
			myPointer = aPointer;
			myGeneration = ResolveGeneration();
		}
	}

	Ptr(T& aPointer)
		: Ptr(&aPointer)
	{
	}

	bool IsValid() const
	{
		if (myPointer)
		{
			if (myGeneration == ResolveGeneration())
				return true;
			*this = Ptr();
		}

		return false;
	}

	operator T*() const
	{
		if (IsValid())
			return myPointer;
		return nullptr;
	}

	T* operator->() const
	{
		return *this;
	}

private:
	u16 ResolveGeneration() const
	{
		if (!myPointer)
			return 0;

		if constexpr (IsDerivedFrom<T, Component>)
		{
			return reinterpret_cast<Component*>(myPointer)->myChunk->FindGeneration(*reinterpret_cast<Component*>(myPointer));
		}
		else if constexpr (IsDerivedFrom<T, Object>)
		{
			return reinterpret_cast<Object*>(myPointer)->myChunk->FindGeneration(*reinterpret_cast<Object*>(myPointer));
		}
		else
		{
			static_assert(false, "Ptr can only be used to point on components and objects");
			abort();
		}
	}

	mutable T* myPointer;
	mutable u16 myGeneration;
};
