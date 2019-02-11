#pragma once

template <typename TBase, size Size = sizeof(TBase)>
class InlineVirtualObject
{
	std::aligned_storage_t<Size> myStorage;
	bool myHasObject = nullptr;

	static_assert(std::has_virtual_destructor_v<TBase>, "Base must has a virtual destructor");

public:
	InlineVirtualObject()
	{
	}

	~InlineVirtualObject()
	{
		Clear();
	}

	InlineVirtualObject(const InlineVirtualObject&) = delete;
	InlineVirtualObject& operator=(const InlineVirtualObject&) = delete;

	InlineVirtualObject(InlineVirtualObject&& aMove) = delete;
	InlineVirtualObject& operator=(InlineVirtualObject&&) = delete;

	template <typename TDerived, typename ...TArgs>
	void Store(TArgs ...aArgs)
	{
		static_assert(sizeof(TDerived) <= Size, "Provided type too large");
		static_assert(IsDerivedFrom<TDerived, TBase>, "Type stored does not derive from base.");
		Clear();
		new (static_cast<void*>(&myStorage)) TDerived(std::forward<TArgs>(aArgs)...);
		myHasObject = true;
	}

	void Clear()
	{
		if (!IsValid())
			return;
		Get()->~TBase();
		myHasObject = false;
	}

	FORCEINLINE bool IsValid() const
	{
		return myHasObject;
	}

	FORCEINLINE TBase* Get() const
	{
		if (!IsValid())
			return nullptr;
		return const_cast<TBase*>(reinterpret_cast<const TBase*>(&myStorage));
	}

	FORCEINLINE TBase* operator->() const
	{
		return Get();
	}

	FORCEINLINE TBase& operator*() const
	{
		return *Get();
	}

	FORCEINLINE operator TBase*() const
	{
		return Get();
	}

	FORCEINLINE explicit operator bool() const
	{
		return IsValid();
	}
};
