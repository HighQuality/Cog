#include "CogPch.h"
#include "ObjectPool.h"
#include "Object.h"
#include "TypeList.h"

UniquePtr<TypeList> RegisterTypeList()
{
	UniquePtr<TypeList> typeList = MakeUnique<TypeList>();
	typeList->BuildList();
	return typeList;
}

static UniquePtr<const TypeList> gTypeList = RegisterTypeList();

static UniquePtr<CogTypeChunk> CreateChunkOfType(const TypeID<Object>& aType)
{
	return gTypeList->GetTypeData(aType).AllocateFactory();
}

Ptr<Object> ObjectPool::CreateObjectByType(const TypeID<Object>& aType, const Ptr<Object>& aOwner)
{
	bool created;
	CogTypeChunk& chunk = FindOrCreateObjectChunk(created, aType, aOwner, &CreateChunkOfType);
	return chunk.Allocate(aOwner, created && !aOwner.IsValid());
}

CogTypeChunk& ObjectPool::FindOrCreateObjectChunk(bool& aCreated, const TypeID<Object>& aObjectType, const Ptr<Object>& aOwner, UniquePtr<CogTypeChunk> (*aChunkCreator)(const TypeID<Object>&))
{
	const u16 index = aObjectType.GetUnderlyingInteger();
	myObjectChunks.Resize(TypeID<Object>::MaxUnderlyingInteger());

	UniquePtr<CogTypeChunk>& chunk = myObjectChunks[index];
	if (!chunk)
	{
		chunk = aChunkCreator(aObjectType);
		if (aOwner)
			chunk->SetProgram(aOwner->GetProgram());
		chunk->Initialize();
		aCreated = true;
	}
	else
	{
		aCreated = false;
	}

	return *chunk;
}
