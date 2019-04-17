#pragma once
#include "Object.h"
#include <Utilities/FactoryChunk.h>

template <typename T>
class Ptr final
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

	FORCEINLINE bool operator==(const Ptr& aOther) const
	{
		return myPointer == aOther.myPointer;
	}

	FORCEINLINE bool operator!=(const Ptr& aOther) const
	{
		return myPointer != aOther.myPointer;
	}

	explicit operator bool() const { return IsValid(); }

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
			return reinterpret_cast<T*>(myPointer);
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

		const Object& component = *reinterpret_cast<const Object*>(myPointer);
		if (const BaseFactoryChunk* chunk = component.myChunk)
			return chunk->FindGeneration(component.myChunkIndex);
		return 0;
	}

	mutable Object* myPointer;
	mutable u16 myGeneration;
};
