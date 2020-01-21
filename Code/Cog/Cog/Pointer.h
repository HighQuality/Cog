#pragma once
#include "Object.h"

// Non-owning weak pointer to a Object, becomes null when object is destroyed
template <typename T>
class Ptr final
{
	static_assert(sizeof(T) == sizeof(Object), "Subclass of Object may not declare member variables");
	
public:
	FORCEINLINE Ptr()
	{
		memset(&myObject, 0, sizeof Object);
	}
	
	FORCEINLINE Ptr(nullptr_t)
		: Ptr()
	{
	}

	FORCEINLINE ~Ptr()
	{
	}

	FORCEINLINE Ptr(T& aReference)
	{
		memcpy(&myObject, &aReference, sizeof Object);
	}

	template <typename TOther, typename = EnableIf<IsDerivedFrom<TOther, T>>>
	FORCEINLINE Ptr(TOther& aReference)
		: Ptr(static_cast<T&>(aReference))
	{
	}

	FORCEINLINE Ptr(T* aPointer)
	{
		if (aPointer)
			memcpy(&myObject, aPointer, sizeof Object);
		else
			memset(&myObject, 0, sizeof Object);
	}

	template <typename TOther, typename = EnableIf<IsDerivedFrom<TOther, T>>>
	FORCEINLINE Ptr(TOther* aPointer)
		: Ptr(static_cast<T*>(aPointer))
	{
	}

	FORCEINLINE Ptr(const Ptr& aOther)
	{
		memcpy(&myObject, &aOther.myObject, sizeof Object);
	}

	template <typename TOther, typename = EnableIf<IsDerivedFrom<TOther, T>>>
	FORCEINLINE Ptr(const Ptr<TOther>& aOther)
	{
		memcpy(&myObject, &aOther.myObject, sizeof Object);
	}

	template <typename TOther>
	FORCEINLINE bool operator==(const Ptr<TOther>& aOther) const
	{
		// Perf: This comparison could skip the vtable
		return memcmp(&myObject, &aOther.myObject, sizeof Object) == 0;
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
		return IsValid() ? static_cast<T*>(&myObject) : nullptr;
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
