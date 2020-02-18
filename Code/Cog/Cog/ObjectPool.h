#pragma once
#include <TypeFundamentals/TypeID.h>

class CogTypeChunk;
class Object;
class CogTypeBase;
class ProgramContext;

template <typename T>
class Ptr;

class ObjectPool
{
public:
	Ptr<Object> CreateObjectByType(const TypeID<CogTypeBase>& aType, const Ptr<Object>& aOwner);
	
	void SetProgramContext(ProgramContext& aProgramContext);

private:
	CogTypeChunk& FindOrCreateObjectChunk(bool& aCreated, const TypeID<CogTypeBase>& aObjectType, const Ptr<Object>& aOwner, UniquePtr<CogTypeChunk>(*aChunkCreator)(const TypeID<CogTypeBase>&));

	Array<UniquePtr<CogTypeChunk>> myObjectChunks;
	ProgramContext* myProgramContext = nullptr;
};
