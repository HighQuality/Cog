#pragma once

template <i32 TSize>
struct InlineObjectSize { static constexpr i32 Size = TSize; };

// This setup should probably be replaced by a runtime NextPowerOfTwo(sizeof(T) + sizeof(void*)) calculation
template <typename T>
auto GetInlineObjectSizeHelper(i32) -> typename T::InlinedSize;

// Leave a pointer's of space left so sizeof(InlineObject) is 32
template <typename T>
auto GetInlineObjectSizeHelper(...) -> InlineObjectSize<32 - sizeof(void*)>;

struct InlineObjectTypeHelper
{
	virtual void ConstructMove(void* aTarget, void* aSource) const = 0;
	virtual void ConstructCopy(void* aTarget, const void* aSource) const = 0;
};

template <typename T>
class InlineObject
{
	static constexpr i32 Size = decltype(GetInlineObjectSizeHelper<T>(0))::Size;
	
	static_assert(std::has_virtual_destructor_v<T>, "Base must have a virtual destructor");
	static_assert(sizeof(T) <= Size, "Increase or specify T::InlinedSize");

public:
	FORCEINLINE ~InlineObject()
	{
		Get().~T();
	}

	InlineObject(const InlineObject& aCopy)
	{
		myTypeHelper = aCopy.myTypeHelper;
		myTypeHelper->ConstructCopy(&Get(), &aCopy);
	}
	
	InlineObject& operator=(const InlineObject& aCopy)
	{
		if (&aCopy == this)
			return;
		
		Get().~T();
		ZeroOurMemory();
		myTypeHelper = aCopy.myTypeHelper;
		myTypeHelper->ConstructCopy(&Get(), &aCopy);
		return *this;
	}

	InlineObject(InlineObject&& aMove)
	{
		ZeroOurMemory();
		myTypeHelper = aMove.myTypeHelper;
		myTypeHelper->ConstructMove(&Get(), &aMove);
	}
	
	InlineObject& operator=(InlineObject&& aMove)
	{
		if (&aMove == this)
			return;

		Get().~T();
		ZeroOurMemory();
		myTypeHelper = aMove.myTypeHelper;
		myTypeHelper->ConstructMove(&Get(), &aMove);
		return *this;
	}

	FORCEINLINE T& Get()
	{
		return *reinterpret_cast<T*>(&myStorage);
	}

	FORCEINLINE const T& Get() const
	{
		return *reinterpret_cast<const T*>(&myStorage);
	}

	FORCEINLINE T* operator->() { return &Get(); }
	FORCEINLINE T& operator*() { return Get(); }

	FORCEINLINE const T* operator->() const { return &Get(); }
	FORCEINLINE const T& operator*() const { return Get(); }

	FORCEINLINE operator T& () { return Get(); }
	FORCEINLINE operator const T&() const { return Get(); }

	template <typename TDerivedType, typename ...TArgs>
	static InlineObject New(TArgs ...aArgs)
	{
		static_assert(sizeof(TDerivedType) <= Size, "Increase or specify InlineObject::T::InlinedSize");
		
		InlineObject obj;
		new (static_cast<void*>(&obj.myStorage)) TDerivedType(Move(aArgs)...);

		static const TypeHelperImpl<TDerivedType> ourTypeHelper;
		obj.myTypeHelper = &ourTypeHelper;
		return obj;
	}
	
private:
	template <typename TDerivedType>
	struct TypeHelperImpl final : InlineObjectTypeHelper
	{
		void ConstructMove(void* aTarget, void* aSource) const override
		{
			if constexpr (std::is_move_constructible_v<TDerivedType>)
				new (aTarget) TDerivedType(Move(*static_cast<TDerivedType*>(aSource)));
			else
				FATAL(L"Object not move constructible");
		}
		
		void ConstructCopy(void* aTarget, const void* aSource) const override
		{
			if constexpr (std::is_copy_constructible_v<TDerivedType>)
				new (aTarget) TDerivedType(*static_cast<const TDerivedType*>(aSource));
			else
				FATAL(L"Object not copy constructible");
		}
	};

	void ZeroOurMemory()
	{
		memset(this, 0, sizeof *this);
	}

	std::aligned_storage_t<Size> myStorage;

	const InlineObjectTypeHelper* myTypeHelper = nullptr;
	
	InlineObject()
	{
		ZeroOurMemory();
	}
};
