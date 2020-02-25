#include "CogPch.h"
#include "ChunkPool.h"
#include "TypeData.h"

ChunkPool::ChunkPool(const TypeData& aTypeData)
	: myTypeData(aTypeData)
{
}

ChunkPool::~ChunkPool()
{
}

void ChunkPool::SetProgramContext(ProgramContext& aProgramContext)
{
	myProgramContext = &aProgramContext;
}

Ptr<Object> ChunkPool::AllocateObject()
{
	scoped_lock(myMutex)
	{
		for (;;)
		{
			for (CogTypeChunk* chunk : myChunks)
			{
				if (Ptr<Object> obj = chunk->Allocate())
					return obj;
			}

			UniquePtr<CogTypeChunk> allocatedChunk = myTypeData.AllocateFactory();
			CogTypeChunk& chunk = *myChunks.Insert(0, Move(allocatedChunk));
			chunk.SetProgramContext(*myProgramContext);
			chunk.Initialize();
		}
	}

	FATAL(L"Unreachable");
}
