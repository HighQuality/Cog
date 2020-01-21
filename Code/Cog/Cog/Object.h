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

	bool IsPendingDestroy() const;

	u8 GetGeneration() const;

	virtual void GetBaseClasses(const FunctionView<void(const TypeID<Object>&)>& aFunction) const { aFunction(TypeID<Object>::Resolve<Object>()); }

	/** Returns false if this instance's memory has been given to another instance. */
	FORCEINLINE bool IsValid() const { return myGeneration == GetGeneration(); }

protected:
	virtual void Destroyed();

	template <typename T>
	friend class Ptr;

	friend class CogTypeChunk;

	// TODO: Try inlining myChunkIndex into myChunk's memory and mask them out as needed, this allows myGeneration to be moved out into Ptr<T> and the padding to be removed thus reducing the size of Object from 24 to 16 bytes
	CogTypeChunk* myChunk = nullptr;
	u8 myChunkIndex = 0;
	u8 myGeneration = 0;

	// Unused memory, feel free to use but I suspect they might not have any use as it's per pointer storage instead of per instance
	u8 _padding2;
	u8 _padding3;
	u8 _padding4;
	u8 _padding5;
	u8 _padding6;
	u8 _padding7;
};
