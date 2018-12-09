#include "pch.h"
#include "HeapAllocator.h"
#include <stdlib.h>
#include <new>
#include "DebugAsserts.h"
#include <limits>

thread_local HeapAllocator gHeap;

struct MemoryBlockHeader
{
	size allocationSize;
	u8 powerOfTwo;
	u8 magicOne = 200;
	u16 magicTwo = 43172;
};

static_assert(((sizeof MemoryBlockHeader) % 16) == 0, "MemoryBlockHeader must be multiple of 16 bytes");

HeapAllocator::HeapAllocator()
{
}

HeapAllocator::~HeapAllocator()
{
}

void * HeapAllocator::Allocate(size aByteAmount)
{
	//CeilToLogTwo(aByteAmount)

	void *mem = malloc(sizeof MemoryBlockHeader + aByteAmount);
	if (!mem)
		abort();
	if (!ENSURE((reinterpret_cast<u64>(mem) % 16) == 0))
		abort();
	new (mem) MemoryBlockHeader();
	return static_cast<void*>(static_cast<u8*>(mem) + sizeof MemoryBlockHeader);
}

static const MemoryBlockHeader DefaultBlockHeader;

void HeapAllocator::Free(void* mm)
{
	MemoryBlockHeader* returnedBlock = static_cast<MemoryBlockHeader*>(static_cast<void*>(static_cast<u8*>(mm) - sizeof MemoryBlockHeader));

	if (returnedBlock->magicOne != DefaultBlockHeader.magicOne || returnedBlock->magicTwo != DefaultBlockHeader.magicTwo)
	{
		abort();
	}
}
