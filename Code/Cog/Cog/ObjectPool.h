#pragma once
#include <TypeFundamentals/TypeID.h>

class Object;
class CogTypeBase;
class ProgramContext;
class ChunkPool;

template <typename T>
class Ptr;

class ObjectPool
{
public:
	ObjectPool();
	~ObjectPool();
	Ptr<Object> CreateObjectByType(const TypeID<CogTypeBase>& aType, const Ptr<Object>& aOwner);
	
	void SetProgramContext(ProgramContext& aProgramContext);

private:
	Array<UniquePtr<ChunkPool>> myChunkPools;
};
