#include "CogPch.h"
#include "ObjectPool.h"
#include "Object.h"
#include "TypeList.h"
#include "ChunkPool.h"

ObjectPool::ObjectPool()
{
	myChunkPools.Resize(TypeID<CogTypeBase>::MaxUnderlyingInteger() + 1);

	for (const TypeData* typeData : gTypeList.GetObjectTypes(false))
		myChunkPools[typeData->GetTypeID().GetUnderlyingInteger()] = MakeUnique<ChunkPool>(*typeData);
}

ObjectPool::~ObjectPool()
{
}

Ptr<Object> ObjectPool::CreateObjectByType(const TypeID<CogTypeBase>& aType, const Ptr<Object>& aOwner)
{
	ChunkPool* chunkPool = myChunkPools[aType.GetUnderlyingInteger()];
	CHECK(chunkPool);
	Ptr<Object> obj = chunkPool->AllocateObject();

	obj->myChunk->SetOwner(obj->myChunkIndex, aOwner);
	obj->myChunk->InitializeObjectAtIndex(obj->myChunkIndex);

	// TODO: This should probably be called from the generated function InitializeObjectAtIndex in order to avoid virtual function call
	obj->myBaseCalled = 0;
	obj->Created();
	CHECK_MSG(obj->myBaseCalled == 1, L"Object subclass did not call Base::Created() all the way down to Object");

	return obj;
}

void ObjectPool::SetProgramContext(ProgramContext& aProgramContext)
{
	for (ChunkPool* chunkPool : myChunkPools)
	{
		if (chunkPool)
			chunkPool->SetProgramContext(aProgramContext);
	}
}
