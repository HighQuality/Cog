#pragma once
#include "Object.h"

// Non-owning weak pointer to a Object, becomes null when object is destroyed
template <typename T>
class Ptr final
{
public:
	FORCEINLINE Ptr()
	{
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

	FORCEINLINE Ptr(T& aPointer)
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

	FORCEINLINE explicit operator bool() const { return IsValid(); }

	bool IsValid() const
	{		 
		return &reinterpret_cast<InlineObject<Object>*>(&myPointer)->Get()->IsValid();
	}

	FORCEINLINE operator T*() const
	{
		if (IsValid())
			return &reinterpret_cast<InlineObject<Object>*>(&myPointer)->Get();
		return nullptr;
	}

	FORCEINLINE T* operator->() const
	{
		return *this;
	}

private:
	mutable InlineObject<Object> myPointer;
};
