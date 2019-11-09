#pragma once

template <typename T>
class UniquePtr
{
public:
	UniquePtr() = default;

	~UniquePtr()
	{
		Clear();
	}

	UniquePtr(const UniquePtr&) = delete;
	UniquePtr& operator=(const UniquePtr&) = delete;

	UniquePtr(UniquePtr&& aMove)
	{
		if (*this == aMove)
			return;

		*this = Move(aMove);
	}

	UniquePtr& operator=(UniquePtr&& aMove)
	{
		if (*this == aMove)
			return *this;

		Clear();
		myObject = Move(aMove.myObject);
		aMove.myObject = nullptr;

		return *this;
	}

	template <typename U>
	UniquePtr(UniquePtr<U>&& aMove)
		: UniquePtr(static_cast<T*>(aMove.Release()))
	{
	}

	template <typename U>
	UniquePtr& operator=(UniquePtr<U>&& aMove)
	{
		*this = UniquePtr(static_cast<T*>(aMove.Release()));
		return *this;
	}

	void Clear()
	{
		delete myObject;
		myObject = nullptr;
	}

	T* Get() const
	{
		return myObject;
	}

	FORCEINLINE explicit operator bool() const { return myObject != nullptr; }
	FORCEINLINE bool IsValid() const { return *this; }

	FORCEINLINE operator T* () const
	{
		return myObject;
	}

	FORCEINLINE T* operator->() const
	{
		return myObject;
	}

	FORCEINLINE T& operator*() const
	{
		return *myObject;
	}

	FORCEINLINE bool operator==(const UniquePtr& aOther) const { return myObject == aOther.myObject; }
	FORCEINLINE bool operator!=(const UniquePtr& aOther) const { return myObject != aOther.myObject; }

	T* Release()
	{
		T* obj = myObject;
		myObject = nullptr;
		return obj;
	}

private:
	template <typename T2, typename ...TArgs>
	friend UniquePtr<T2> MakeUnique(TArgs ...aArgs);

	UniquePtr(T* aPtr)
	{
		myObject = aPtr;
	}

	T* myObject = nullptr;
};

template <typename T, typename ...TArgs>
UniquePtr<T> MakeUnique(TArgs ...aArgs)
{
	return UniquePtr<T>(new T(Move(aArgs)...));
}
