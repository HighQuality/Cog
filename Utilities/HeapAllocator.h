#pragma once
#include "Types.h"
#include <vector>
#include <stack>

struct MemoryBlockHeader;

class HeapAllocator
{
public:
	HeapAllocator();
	~HeapAllocator();

	void * Allocate(size aByteAmount);
	void Free(void* mm);

private:
	std::vector<std::stack<MemoryBlockHeader*>> myMemoryBlocks;
};

extern thread_local HeapAllocator gHeap;
