#pragma once

template <typename T>
class ManualInitializationObject
{
public:
	template <typename ...TArgs>
	FORCEINLINE void Construct(TArgs ...aArgs)
	{
		new (static_cast<void*>(myData)) T(Move(aArgs)...);
	}

	FORCEINLINE void Destruct()
	{
		Get().~T();
	}

	FORCEINLINE T& Get()
	{
		// TODO: Is this UB without std::launder?
		return *reinterpret_cast<T*>(myData);
	}

	FORCEINLINE const T& Get() const
	{
		// TODO: Is this UB without std::launder?
		return *reinterpret_cast<const T*>(myData);
	}

	FORCEINLINE operator T&()
	{
		return Get();
	}

	FORCEINLINE operator const T&() const
	{
		return Get();
	}

	FORCEINLINE T& operator*()
	{
		return Get();
	}

	FORCEINLINE const T& operator*() const
	{
		return Get();
	}

	FORCEINLINE T* operator->()
	{
		return Get();
	}

	FORCEINLINE const T* operator->() const
	{
		return Get();
	}

private:
	alignas(T) u8 myData[sizeof(T)];
};
