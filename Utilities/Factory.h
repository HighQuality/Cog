#pragma once
#include "BaseFactory.h"
#include "FactoryChunk.h"

template <typename T, typename TChunkType = FactoryChunk<T>>
class FactoryImplementation : public BaseFactory
{
	using Chunk = TChunkType;

public:
	FactoryImplementation()
	{
		myChunks.Add(new Chunk(4));
	}

	template <typename TCallback>
	void ForEach(const TCallback& callback) const
	{
		for (Chunk* chunk : myChunks)
			chunk->ForEach(callback);
	}

	void ReturnAll()
	{
		for (Chunk* chunk : myChunks)
			chunk->ReturnAll();
	}
	
	virtual T& Allocate()
	{
		for (Chunk* chunk : myChunks)
		{
			if (!chunk->IsFull())
				return chunk->Allocate();
		}

		const u16 oldMaxSize = myChunks.Last()->GetSize();
		Chunk* newChunk;
		
		if (oldMaxSize << 4 > oldMaxSize)
			newChunk = new Chunk(oldMaxSize << 4);
		else
			newChunk = new Chunk(oldMaxSize);

		myChunks.Add(newChunk);
		return newChunk->Allocate();
	}
	
	virtual void Return(const T& object)
	{
		for (Chunk* chunk : myChunks)
		{
			if (chunk->DoesObjectOriginateFromHere(object))
			{
				chunk->Return(object);
				return;
			}
		}

		FATAL(L"Object does not originate from us.");
	}

	virtual ~FactoryImplementation()
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

// Specialize this
template <typename T>
using Factory = FactoryImplementation<T, FactoryChunk<T>>;
