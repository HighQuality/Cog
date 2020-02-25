#pragma once

class TypeData;
class CogTypeChunk;

class ChunkPool
{
public:
	ChunkPool(const TypeData& aTypeData);
	~ChunkPool();

	Ptr<Object> AllocateObject();

	void SetProgramContext(ProgramContext& aProgramContext);

private:
	Array<UniquePtr<CogTypeChunk>> myChunks;
	std::mutex myMutex;
	ProgramContext* myProgramContext = nullptr;
	const TypeData& myTypeData;
};
