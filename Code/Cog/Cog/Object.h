#pragma once
#include "CogTypeChunk.h"
#include "Object.generated.h"

COGTYPE()
class Object
{
	GENERATED_BODY;
	
public:
	using Base = void;

	Object();
	virtual ~Object();
	
	DELETE_COPYCONSTRUCTORS_AND_MOVES(Object);

	virtual bool Destroy();

	void ReturnToAllocator();

	FORCEINLINE bool IsPendingDestroy() const { return GetChunk()->IsPendingDestroy(myChunkIndex); }

	virtual void GetBaseClasses(const FunctionView<void(const TypeID<Object>&)>& aFunction) const { aFunction(TypeID<Object>::Resolve<Object>()); }

protected:
	virtual void Destroyed();

	template <typename T>
	friend class Ptr;

	template <typename T>
	friend class FactoryChunk;

	friend class CogGame;

	CogTypeChunk* myChunk;
	u8 myChunkIndex;
};
