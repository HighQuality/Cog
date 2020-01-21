#pragma once
#include "Object.h"

// Non-owning weak pointer to a Object, becomes null when object is destroyed
template <typename T>
class Ptr final
{
	static_assert(sizeof(T) == sizeof(Object), "Derived class from Object may not declare member variables");
	
public:
	FORCEINLINE Ptr()
	{
		memset(&myObject, 0, sizeof T);
	}

	Ptr(T* aPointer)
	{
		if (aPointer)
			memcpy(&myObject, aPointer, sizeof T);
		else
			memset(&myObject, 0, sizeof T);
	}

	FORCEINLINE Ptr(T& aPointer)
	{
		memcpy(&myObject, &aPointer, sizeof T);
	}

	Ptr(const Ptr& aOther)
	{
		memcpy(&myObject, &aOther.myObject, sizeof T);
	}

	~Ptr()
	{
#ifdef _DEBUG
		myObject.myChunk = nullptr;
#endif
	}
	
	template <typename TOther>
	bool operator==(const Ptr<TOther>& aOther) const
	{
		// Perf: This comparison could skip the vtable
		return memcmp(&myObject, &aOther.myObject, sizeof T) == 0;
	}

	template <typename TOther>
	FORCEINLINE bool operator!=(const Ptr<TOther>& aOther) const
	{
		return !(*this == aOther);
	}

	FORCEINLINE explicit operator bool() const { return IsValid(); }

	FORCEINLINE bool IsValid() const
	{
		return myObject.myChunk && myObject.IsValid();
	}
	
	FORCEINLINE T* Get() const
	{
		return IsValid() ? &myObject : nullptr;
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
	template <typename TOther>
	friend class Ptr;

	// Wrapped in union in order to avoid construction of Object
	union
	{
		mutable Object myObject;
	};
};
