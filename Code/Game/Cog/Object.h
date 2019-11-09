#pragma once
#include "InheritType.h"

class BaseFactoryChunk;

class Object
{
public:
	using Base = void;

	Object();
	virtual ~Object();
	
	// void ReceiveRawMessage(void* aMessage, const TypeID<MessageSystem::Message>& aTypeId) const;

	DELETE_COPYCONSTRUCTORS_AND_MOVES(Object);

	virtual bool Destroy();

	void ReturnToAllocator();

	FORCEINLINE bool IsActivated() const;
	FORCEINLINE bool IsPendingDestroy() const;

	template <typename T>
	void SendMessage(T aMessage)
	{

	}

	virtual void GetBaseClasses(const FunctionView<void(const TypeID<Object>&)>& aFunction) const
	{
		aFunction(TypeID<Object>::Resolve<Object>());
	}

protected:
	virtual void Destroyed();

	void SetActivated(bool aIsActivated);

	template <typename T>
	friend class Ptr;

	template <typename T>
	friend class FactoryChunk;

	friend class CogGame;


	BaseFactoryChunk* myChunk;
	ChunkIndex myChunkIndex;
};

#include <Memory/FactoryChunk.h>

FORCEINLINE bool Object::IsActivated() const { return myChunk->IsActivated(myChunkIndex); }
FORCEINLINE bool Object::IsPendingDestroy() const { return myChunk->IsPendingDestroy(myChunkIndex); }