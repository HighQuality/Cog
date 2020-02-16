#pragma once
#include <TypeFundamentals/TypeID.h>

class CogTypeChunk;
class Object;

template <typename T>
class Ptr;

class ObjectPool
{
public:
	Ptr<Object> CreateObjectByType(const TypeID<Object>& aType, const Ptr<Object>& aOwner);
	
private:
	CogTypeChunk& FindOrCreateObjectChunk(bool& aCreated, const TypeID<Object>& aObjectType, const Ptr<Object>& aOwner, UniquePtr<CogTypeChunk>(*aChunkCreator)(const TypeID<Object>&));

	Array<UniquePtr<CogTypeChunk>> myObjectChunks;
};
