#pragma once

class FiberHandle
{
public:
	FiberHandle()
	{
		myHandle = nullptr;
	}

	explicit FiberHandle(void* aHandle)
	{
		myHandle = aHandle;
	}

	FORCEINLINE void* GetHandle() const { return myHandle; }

	FORCEINLINE bool IsValid() const { return myHandle != nullptr; }

	void Reset()
	{
		myHandle = nullptr;
	}

	FORCEINLINE bool operator==(const FiberHandle& aOther) const
	{
		return myHandle == aOther.myHandle;
	}

	FORCEINLINE bool operator!=(const FiberHandle& aOther) const
	{
		return myHandle != aOther.myHandle;
	}

	FORCEINLINE explicit operator bool() const
	{
		return myHandle != nullptr;
	}

private:
	void* myHandle;
};
