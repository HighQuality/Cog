#pragma once
#include <TypeFundamentals/TypeID.h>

class CogTypeChunk;
class Object;

template <typename T>
class Ptr;

class ObjectPool
{
public:
	Ptr<Object> CreateObjectByType(const TypeID<Object>& aType);

private:
	CogTypeChunk& FindOrCreateObjectChunk(const TypeID<Object>& aObjectType, UniquePtr<CogTypeChunk>(*aChunkCreator)(const TypeID<Object>&));

	Array<UniquePtr<CogTypeChunk>> myObjectChunks;
};
