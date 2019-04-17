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
		myChunks.Add(&myFirstChunk);
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

		Chunk* newChunk = new Chunk();
		myChunks.Add(newChunk);
		return newChunk->Allocate();
	}

	void Return(const T & aObject)
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
		// First chunk is not heap allocated
		for (i32 i = 1; i < myChunks.GetLength(); ++i)
			delete myChunks[i];
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
	Chunk myFirstChunk;
	Array<Chunk*> myChunks;
};
