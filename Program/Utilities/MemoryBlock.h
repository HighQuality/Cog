#pragma once
#include "Types.h"
#include "Optional.h"

template <typename T>
class MemoryBlock
{
public:
	MemoryBlock()
	{
		myFreeCounter = 256;
		for (size i = 0; i < 256; ++i)
			myFreeIndex[i] = true;
	}

	~MemoryBlock()
	{
		for (size i = 0; i < 256; ++i)
		{
			if (myFreeIndex[i] == false)
				reinterpret_cast<T*>(myMemory)[i]->~T();
		}
		myFreeCounter = 256;
	}

	bool IsFull() const
	{
		return myFreeCounter == 0;
	}

	bool IsEmpty() const
	{
		return myFreeCounter == 256;
	}

	Optional<T*> Allocate()
	{
		if (!ENSURE(!IsFull()))
			return Optional<T*>::Empty;

		T* NewObject = nullptr;

		for (size i = 0; i < 256; ++i)
		{
			if (myFreeIndex[i] == true)
			{
				NewObject = reinterpret_cast<T*>(myMemory)[i];
				myFreeIndex[i] = false;
			}
		}

		if (!ENSURE(NewObject))
			return Optional<T*>::Empty;

		new (NewObject) T();
		return Optional<T*>::MakeWithValue(NewObject);
	}

	void Free(T * aObject)
	{
		// Out of range, might be a good idea to crash? @ep
		if (!ENSURE(Object >= reinterpret_cast<T*>(myMemory) && Object < (reinterpret_cast<T*>(myMemory) + 255)))
			return;
		
		size index = static_cast<size>(Object - reinterpret_cast<T*>(myMemory));
		
		Object->~T();

		myFreeIndex[index] = true;
		++myFreeCounter;
	}

	void FreeIndex(const size aIndex)
	{
		if (!ENSURE(aIndex >= 0 && aIndex < 256))
			return;

		reinterpret_cast<T*>(myMemory)[aIndex]->~T();
		myFreeIndex[aIndex] = true;
		++myFreeCounter;
	}

private:
	u8 myMemory[256 * sizeof T];
	bool myFreeIndex[256];
	u16 myFreeCounter;
};
