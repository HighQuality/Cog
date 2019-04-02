#pragma once
#include "MessageSystem.h"
#include <BackgroundWorkAwaitable.h>

class BaseFactoryChunk;



class Object
{
public:
	using Base = void;

	Object();
	virtual ~Object();
	
	// void ReceiveRawMessage(void* aMessage, const TypeID<MessageSystem::Message>& aTypeId) const;

	DELETE_COPYCONSTRUCTORS_AND_MOVES(Object);

	virtual void Destroy();
	
	FORCEINLINE bool IsActivated() const;
	FORCEINLINE bool IsPendingDestroy() const;

	template <typename T>
	void SendMessage(T aMessage)
	{

	}

protected:
	virtual void Destroyed();

	void SetActivated(bool aIsActivated);

	template <typename T>
	friend class Ptr;

	template <typename T>
	friend class FactoryChunk;

	friend class CogGame;

	void ReturnToAllocator();

	BaseFactoryChunk* myChunk;
	u16 myChunkIndex;
};

#include "FactoryChunk.h"

FORCEINLINE bool Object::IsActivated() const { return myChunk->IsActivated(myChunkIndex); }
FORCEINLINE bool Object::IsPendingDestroy() const { return myChunk->IsPendingDestroy(myChunkIndex); }
