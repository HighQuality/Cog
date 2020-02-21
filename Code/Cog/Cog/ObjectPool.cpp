#include "CogPch.h"
#include "ObjectPool.h"
#include "Object.h"
#include "TypeList.h"

static UniquePtr<CogTypeChunk> CreateChunkOfType(const TypeID<CogTypeBase>& aType)
{
	return gTypeList.GetTypeData(aType).AllocateFactory();
}

Ptr<Object> ObjectPool::CreateObjectByType(const TypeID<CogTypeBase>& aType, const Ptr<Object>& aOwner)
{
	bool created;
	CogTypeChunk& chunk = FindOrCreateObjectChunk(created, aType, aOwner, &CreateChunkOfType);
	if (created)
		chunk.SetProgramContext(*myProgramContext);
	return chunk.Allocate(aOwner);
}

void ObjectPool::SetProgramContext(ProgramContext& aProgramContext)
{
	myProgramContext = &aProgramContext;
}

CogTypeChunk& ObjectPool::FindOrCreateObjectChunk(bool& aCreated, const TypeID<CogTypeBase>& aObjectType, const Ptr<Object>& aOwner, UniquePtr<CogTypeChunk> (*aChunkCreator)(const TypeID<CogTypeBase>&))
{
	const u16 index = aObjectType.GetUnderlyingInteger();
	myObjectChunks.Resize(TypeID<CogTypeBase>::MaxUnderlyingInteger());

	UniquePtr<CogTypeChunk>& chunk = myObjectChunks[index];
	if (!chunk)
	{
		chunk = aChunkCreator(aObjectType);
		chunk->SetProgramContext(*myProgramContext);
		chunk->Initialize();
		aCreated = true;
	}
	else
	{
		aCreated = false;
	}

	return *chunk;
}
