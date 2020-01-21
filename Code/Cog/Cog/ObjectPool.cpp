#include "CogPch.h"
#include "ObjectPool.h"
#include "Object.h"
#include "TypeList.h"

static ObjectPool gObjectPool;
static UniquePtr<const TypeList> gTypeList;

void RegisterTypeList()
{
	UniquePtr<TypeList> typeList = MakeUnique<TypeList>();
	typeList->BuildList();
	gTypeList = Move(typeList);
}

Ptr<Object> NewObjectByType(const TypeID<Object>& aTypeID)
{
	return gObjectPool.CreateObjectByType(aTypeID);
}

static UniquePtr<CogTypeChunk> CreateChunkOfType(const TypeID<Object>& aType)
{
	return gTypeList->GetTypeData(aType).AllocateFactory();
}

Ptr<Object> ObjectPool::CreateObjectByType(const TypeID<Object>& aType)
{
	CogTypeChunk& factory = FindOrCreateObjectChunk(aType, &CreateChunkOfType);

	return factory.Allocate();
}

CogTypeChunk& ObjectPool::FindOrCreateObjectChunk(const TypeID<Object>& aObjectType, UniquePtr<CogTypeChunk> (*aChunkCreator)(const TypeID<Object>&))
{
	const u16 index = aObjectType.GetUnderlyingInteger();
	myObjectChunks.Resize(TypeID<Object>::MaxUnderlyingInteger());
	auto& chunk = myObjectChunks[index];
	if (!chunk)
	{
		chunk = aChunkCreator(aObjectType);
		chunk->Initialize();
	}
	return *chunk;
}
