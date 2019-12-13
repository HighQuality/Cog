#pragma once
#include "CogTypeChunk.h"
#include <Memory/InlineObject.h>
#include "Object.generated.h"

COGTYPE()
class Object
{
	GENERATED_BODY;
	
public:
	using InlinedSize = InlineObjectSize<24>;
	
	using Base = void;

	Object();
	virtual ~Object();
	
	DELETE_COPYCONSTRUCTORS_AND_MOVES(Object);

	virtual bool Destroy();

	void ReturnToAllocator();

	FORCEINLINE bool IsPendingDestroy() const { return GetChunk().IsPendingDestroy(myChunkIndex); }

	FORCEINLINE u8 GetGeneration() const { return GetChunk().GetGeneration(myChunkIndex); }
	
	virtual void GetBaseClasses(const FunctionView<void(const TypeID<Object>&)>& aFunction) const { aFunction(TypeID<Object>::Resolve<Object>()); }

	/** Returns false if this instance's memory has been given to another instance. */
	FORCEINLINE bool IsValid() const { return myGeneration == GetGeneration(); }
	
protected:
	virtual void Destroyed();

	template <typename T>
	friend class Ptr;

	// TODO: Try inlining myChunkIndex into myChunk's memory and mask them out as needed, this allows myGeneration to be moved out into Ptr<T>
	CogTypeChunk* myChunk;
	u8 myChunkIndex;
	u8 myGeneration;

	// Unused memory, feel free to use but I suspect they might not have any use as it's per pointer storage instead of per instance
	u8 _padding2;
	u8 _padding3;
	u8 _padding4;
	u8 _padding5;
	u8 _padding6;
	u8 _padding7;
};
