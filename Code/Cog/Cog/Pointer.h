#pragma once
#include "Object.h"

// Non-owning weak pointer to a Object, becomes null when object is destroyed
template <typename T>
class Ptr final
{
public:
	FORCEINLINE Ptr()
	{
		Read()->myChunk = nullptr;
	}
	
	FORCEINLINE Ptr(nullptr_t)
		: Ptr()
	{
	}

	FORCEINLINE ~Ptr()
	{
		Read()->myChunk = nullptr;
	}

	FORCEINLINE Ptr(T* aPointer)
	{
		if (aPointer)
			memcpy(&myObject.Get(), aPointer, sizeof Object);
		else
			memset(&myObject.Get(), 0, sizeof Object);
	}

	template <typename TOther, typename = EnableIf<IsDerivedFrom<TOther, T>>>
	FORCEINLINE Ptr(TOther* aPointer)
		: Ptr(static_cast<T*>(aPointer))
	{
	}

	FORCEINLINE Ptr(const Ptr& aOther)
	{
		memcpy(&myObject.Get(), &aOther.myObject.Get(), sizeof Object);
	}

	template <typename TOther, typename = EnableIf<IsDerivedFrom<TOther, T>>>
	FORCEINLINE Ptr(const Ptr<TOther>& aOther)
	{
		memcpy(&myObject.Get(), &aOther.myObject.Get(), sizeof Object);
	}

	template <typename TOther>
	FORCEINLINE bool operator==(const Ptr<TOther>& aOther) const
	{
		// Perf: This comparison could skip the vtable
		return memcmp(&myObject.Get(), &aOther.myObject.Get(), sizeof Object) == 0;
	}

	template <typename TOther>
	FORCEINLINE bool operator!=(const Ptr<TOther>& aOther) const
	{
		return !(*this == aOther);
	}

	FORCEINLINE explicit operator bool() const { return IsValid(); }

	FORCEINLINE bool IsValid() const
	{
		const Object* object = Read();
		return object->myChunk && object->IsValid();
	}
	
	FORCEINLINE T* Get() const
	{
		return IsValid() ? reinterpret_cast<T*>(Read()) : nullptr;
	}

	FORCEINLINE operator T*() const
	{
		return Get();
	}

	FORCEINLINE T* operator->() const
	{
		// TODO: This should probably be compiled out in shipping builds?
		CHECK(IsValid());
		return *this;
	}

private:
	FORCEINLINE Object* Read() const { return &myObject.Get(); }

	friend class CogTypeChunk;

	template <typename TOther>
	friend class Ptr;

	mutable ManualInitializationObject<Object> myObject;
};
