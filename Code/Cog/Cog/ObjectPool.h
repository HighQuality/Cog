#pragma once
#include <TypeFundamentals/TypeID.h>

class CogTypeChunk;
class Object;
class CogTypeBase;

template <typename T>
class Ptr;

class ObjectPool
{
public:
	Ptr<Object> CreateObjectByType(const TypeID<CogTypeBase>& aType, const Ptr<Object>& aOwner);
	
	void SetProgram(Program& aProgram);

private:
	CogTypeChunk& FindOrCreateObjectChunk(bool& aCreated, const TypeID<CogTypeBase>& aObjectType, const Ptr<Object>& aOwner, UniquePtr<CogTypeChunk>(*aChunkCreator)(const TypeID<CogTypeBase>&));

	Array<UniquePtr<CogTypeChunk>> myObjectChunks;
	Program* myProgram;
};
