#pragma once
#include "BaseFactory.h"
#include "FactoryChunk.h"

template <typename T, typename TChunkType = FactoryChunk<T>>
class Factory : public BaseFactory
{
	using Chunk = TChunkType;

public:
	using Base = BaseFactory;

	Factory()
	{
		myChunks.Add(new Chunk(4));
	}

	template <typename TCallback>
	void ForEach(const TCallback& aCallback) const
	{
		for (Chunk* chunk : myChunks)
			chunk->ForEach(aCallback);
	}

	void ReturnAll() final
	{
		for (Chunk* chunk : myChunks)
			chunk->ReturnAll();
	}

	template <typename TCallback>
	void IterateChunks(TCallback aCallback)
	{
		for (Chunk* chunk : myChunks)
			aCallback(*chunk);
	}
	
	T& Allocate()
	{
		for (Chunk* chunk : myChunks)
		{
			if (!chunk->IsFull())
				return chunk->Allocate();
		}

		const u16 oldMaxSize = myChunks.Last()->GetSize();
		Chunk* newChunk;

		const i32 grownSize = oldMaxSize << 4;

		if (grownSize > oldMaxSize)
			newChunk = new Chunk(grownSize);
		else
			newChunk = new Chunk(oldMaxSize);

		myChunks.Add(newChunk);
		return newChunk->Allocate();
	}
	
	void Return(const T& aObject)
	{
		for (Chunk* chunk : myChunks)
		{
			if (chunk->DoesObjectOriginateFromHere(aObject))
			{
				chunk->Return(aObject);
				return;
			}
		}

		FATAL(L"Object does not originate from us.");
	}

	~Factory()
	{
		for (Chunk* chunk : myChunks)
			delete chunk;
		myChunks.Clear();
	}
	
	void* AllocateRawObject() final
	{
		return &Allocate();
	}

	void ReturnRawObject(const void* aObject) final
	{
		Return(*static_cast<const T*>(aObject));
	}

protected:
	Array<Chunk*> myChunks;
};
