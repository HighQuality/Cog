#pragma once
#include "CogTypeChunk.h"
#include "CogTypeBase.h"
#include "Object.generated.h"

COGTYPE()
class Object : public CogTypeBase
{
	GENERATED_BODY;
	
public:
	virtual bool Destroy();

	void ReturnToAllocator();

	bool IsPendingDestroy() const;

	u8 GetGeneration() const;

	virtual void GetBaseClasses(const FunctionView<void(const TypeID<CogTypeBase>&)>& aFunction) const { aFunction(TypeID<CogTypeBase>::Resolve<Object>()); }

	Program& GetProgram() const final;
	const Ptr<Object>& GetOwner() const;

	template <typename T>
	Ptr<T> NewChild(Class<T> aClass = Class<T>())
	{
		return reinterpret_cast<T*>(NewChildByType(aClass.GetTypeID()).Get());
	}

	Ptr<Object> NewChildByType(const TypeID<CogTypeBase>& aType);

	/** Returns false if this instance's memory has been given to another instance. */
	FORCEINLINE bool IsValid() const { return myGeneration == GetGeneration(); }

protected:
	virtual void Created();
	virtual void Destroyed();

	template <typename T>
	friend class Ptr;

	friend class CogTypeChunk;
	friend class ObjectPool;

	void SetOwner(const Ptr<Object>& aNewOwner);

	// TODO: Try inlining myChunkIndex into myChunk's memory and mask them out as needed, this allows myGeneration to be moved out into Ptr<T> and the padding to be removed thus reducing the size of Object from 24 to 16 bytes
	CogTypeChunk* myChunk;
	u8 myChunkIndex;
	u8 myGeneration;
	
	// Set to 0 and then check if it's 1 to ensure all overriden functions have been called
	u8 myBaseCalled;

	// Unused memory, feel free to use but I suspect they might not have any use as it's per pointer storage instead of per instance
	u8 _padding3;

#ifdef ENV64
	u8 _padding4;
	u8 _padding5;
	u8 _padding6;
	u8 _padding7;
#endif
};
