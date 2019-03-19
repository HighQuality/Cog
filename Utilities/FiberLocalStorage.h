#pragma once

template <typename T>
class FiberLocalStorage
{
public:
	FiberLocalStorage()
	{
		myHandle = FlsAlloc(&FiberLocalStorage::FiberExiting);
		// Out of storage space
		CHECK(myHandle != FLS_OUT_OF_INDEXES);
	}
	
	DELETE_MOVES(FiberLocalStorage);

	template <typename ...TArgs>
	T& Allocate(TArgs ...aArgs)
	{
		T* value = new T(std::forward<TArgs>(aArgs)...);
		FlsSetValue(myHandle, value);
		return *value;
	}

	T& Get()
	{
		T* object = static_cast<T*>(FlsGetValue(myHandle));
		return *object;
	}

	~FiberLocalStorage()
	{
		FlsFree(myHandle);
	}

	operator T&()
	{
		return Get();
	}

private:
	static void FiberExiting(void* aDataPtr)
	{
		delete static_cast<T*>(aDataPtr);
	}

	u32 myHandle = FLS_OUT_OF_INDEXES;
};
