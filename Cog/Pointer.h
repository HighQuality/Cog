#pragma once
#include "Object.h"
#include "Component.h"
#include "ObjectFactory.h"
#include "BaseComponentFactoryChunk.h"

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
			*const_cast<Ptr*>(this) = Ptr();
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
			auto chunk = reinterpret_cast<Component*>(myPointer)->myChunk;
			if (!chunk)
				return 0;
			return chunk->FindGeneration(*reinterpret_cast<Component*>(myPointer));
		}
		else if constexpr (IsDerivedFrom<T, Object>)
		{
			auto chunk = reinterpret_cast<Object*>(myPointer)->myChunk;
			if (!chunk)
				return 0;
			return chunk->FindGeneration(*reinterpret_cast<Object*>(myPointer));
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
