#pragma once
#include "ArraySlice.h"

template <typename TInheritance, typename TElementType>
class ArrayHeapAllocator : public TInheritance
{
	using Super = TInheritance;

protected:
	FORCEINLINE void* AllocateRaw(const i32 aElementCount, i32& outAllocatedElements)
	{
		void* memory = malloc(aElementCount * sizeof TElementType);
		if (!memory)
			abort();
		outAllocatedElements = aElementCount;
		return memory;
	}

	FORCEINLINE void FreeRaw(void* aBlock)
	{
		free(aBlock);
	}
};

template <typename T, typename TAllocator = ArrayHeapAllocator<ArraySlice<T>, T>, bool ZeroOnePastEnd = false>
class Array : public TAllocator
{
public:
	using Super = TAllocator;

	FORCEINLINE Array()
		: Super(*this)
	{
	}

	FORCEINLINE Array(std::initializer_list<T> aList)
		: Array(ArrayView<T>(aList.begin(), static_cast<i32>(aList.size())))
	{
	}

	FORCEINLINE Array(Array&& aMove) noexcept
	{
		this->myData = aMove.myData;
		aMove.myData = nullptr;

		this->myLength = aMove.myLength;
		aMove.myLength = 0;

		this->myCapacity = aMove.myCapacity;
		aMove.myCapacity = 0;
	}

	FORCEINLINE Array(const Array& aCopy)
		: Super(*this)
	{
		this->myData = nullptr;
		this->myLength = 0;
		myCapacity = 0;

		*this = aCopy.View();
	}

	FORCEINLINE explicit Array(const ArrayView<T>& aCopy)
		: Super(*this)
	{
		this->myData = nullptr;
		this->myLength = 0;
		myCapacity = 0;

		*this = aCopy;
	}

	// TODO: Rewrite to support non-default constructible types
	template <typename U>
	explicit Array(const ArrayView<U>& aCopy)
		: Super()
	{
		const i32 length = aCopy.GetLength();

		Resize(length);

		for (i32 i = 0; i < length; ++i)
			(*this)[i] = aCopy[i];
	}

	FORCEINLINE Array& operator=(const Array& aCopy)
	{
		*this = static_cast<const ArraySlice<T>&>(aCopy);
		return *this;
	}

	FORCEINLINE Array& operator=(Array&& aMove) noexcept
	{
		Clear();

		this->myData = aMove.myData;
		aMove.myData = nullptr;
		this->myLength = aMove.myLength;
		aMove.myLength = 0;
		this->myCapacity = aMove.myCapacity;
		aMove.myCapacity = 0;

		return *this;
	}

	FORCEINLINE Array& operator=(const ArrayView<T>& aCopy)
	{
		ClearAndSetCapacity(aCopy.GetLength());

		this->myLength = aCopy.GetLength();

		const T* otherData= aCopy.GetData();

		for (i32 i = 0; i < this->myLength; ++i)
			new (static_cast<void*>(&this->myData[i])) T(otherData[i]);

		return *this;
	}

	FORCEINLINE ~Array()
	{
		Clear();
	}

	FORCEINLINE T& Add(const T & aValue)
	{
		return Emplace(T(aValue));
	}

	FORCEINLINE T& Add(T && aValue)
	{
		return Emplace(Move(aValue));
	}

	FORCEINLINE T& Insert(const i32 aIndex, const T& aValue)
	{
		if (aIndex < 0 || aIndex > this->myLength)
			abort();

		if (aIndex == this->myLength)
			return Add(aValue);

		PrepareAdd(1);

		new (static_cast<void*>(&this->myData[this->myLength - 1])) T(Move(this->myData[this->myLength]));

		for (i32 i = this->myLength - 2; i >= aIndex; --i)
			this->myData[i + 1] = Move(this->myData[i]);

		this->myData[aIndex].~T();

		++this->myLength;
		new (static_cast<void*>(&this->myData[aIndex])) T(aValue);

		return this->myData[aIndex];
	}

	FORCEINLINE T& Insert(const i32 aIndex, T&& aValue)
	{
		if (aIndex < 0 || aIndex > this->myLength)
			abort();

		if (aIndex == this->myLength)
			return Emplace(Move(aValue));

		PrepareAdd(1);

		new (static_cast<void*>(&this->myData[this->myLength - 1])) T(Move(this->myData[this->myLength]));

		for (i32 i = this->myLength - 2; i >= aIndex; --i)
			this->myData[i + 1] = Move(this->myData[i]);

		this->myData[aIndex].~T();
		
		++this->myLength;
		new (static_cast<void*>(&this->myData[aIndex])) T(Move(aValue));

		return this->myData[aIndex];
	}

	FORCEINLINE T& AddUnique(const T & aValue)
	{
		return AddUnique(T(aValue));
	}

	FORCEINLINE T& AddUnique(T && aValue)
	{
		for (i32 i = 0; i < this->myLength; ++i)
		{
			if (this->myData[i] == aValue)
				return this->myData[i];
		}
		return Emplace(Move(aValue));
	}

	FORCEINLINE T& Emplace(T && aValue)
	{
		MakeSizeFor(this->myLength + 1);
		new (static_cast<void*>(&this->myData[this->myLength])) T(Move(aValue));
		++this->myLength;

		if constexpr (ZeroOnePastEnd)
			memset(&this->myData[this->myLength], 0, sizeof T);

		return this->Last();
	}

	FORCEINLINE void Append(const ArrayView<T> & aArray)
	{
		if (aArray.GetLength() == 0)
			return;
		MakeSizeFor(this->myLength + aArray.GetLength());
		for (i32 i = 0; i < aArray.GetLength(); ++i)
			new (static_cast<void*>(&this->myData[this->myLength + i])) T(aArray.GetData()[i]);
		this->myLength += aArray.GetLength();

		if constexpr (ZeroOnePastEnd)
			memset(&this->myData[this->myLength], 0, sizeof T);
	}

	FORCEINLINE bool Remove(const T & aValue, bool bOnlyOne = true)
	{
		bool removedElement = false;

		for (i32 i = 0; i < this->myLength; ++i)
		{
			if ((*this)[i] == aValue)
			{
				RemoveAt(i);
				--i;
				removedElement = true;

				if (bOnlyOne)
					break;
			}
		}

		return removedElement;
	}

	FORCEINLINE bool RemoveSwap(const T & aValue, bool bOnlyOne = true)
	{
		bool removedElement = false;

		for (i32 i = 0; i < this->myLength; ++i)
		{
			if ((*this)[i] == aValue)
			{
				RemoveAtSwap(i);
				--i;
				removedElement = true;

				if (bOnlyOne)
					break;
			}
		}

		return removedElement;
	}

	FORCEINLINE T RemoveAtSwap(const i32 aIndex)
	{
		if (aIndex < 0 || aIndex >= this->myLength)
			abort();

		if (aIndex + 1 == this->myLength)
		{
			return RemoveAt(aIndex);
		}

		this->myData[aIndex] = Move(this->Last());
		--this->myLength;

		T removedData = Move(this->myData[this->myLength]);
		this->myData[this->myLength].~T();

		if constexpr (ZeroOnePastEnd)
			memset(&this->myData[this->myLength], 0, sizeof T);

		return removedData;
	}

	FORCEINLINE T RemoveAt(const i32 aIndex)
	{
		if (aIndex < 0 || aIndex >= this->myLength)
			abort();

		if (aIndex + 1 == this->myLength)
		{
			T removedData = Move(this->myData[aIndex]);
			this->myData[aIndex].~T();
			--this->myLength;

			if constexpr (ZeroOnePastEnd)
				memset(&this->myData[this->myLength], 0, sizeof T);

			return removedData;
		}

		T removedData = Move(this->myData[aIndex]);

		for (i32 i = aIndex + 1; i < this->myLength; ++i)
			this->myData[i - 1] = Move(this->myData[i]);
		
		--this->myLength;
		
		this->myData[this->myLength].~T();

		if constexpr (ZeroOnePastEnd)
			memset(&this->myData[this->myLength], 0, sizeof T);

		return removedData;
	}

	FORCEINLINE i32 GetCapacity() const
	{
		return myCapacity;
	}

	FORCEINLINE void Clear()
	{
		for (i32 i = 0; i < this->myLength; ++i)
			this->myData[i].~T();

		this->FreeRaw(this->myData);
		this->myData = nullptr;
		this->myLength = 0;
		myCapacity = 0;
	}

	// Clear without changing memory allocations
	void Empty()
	{
		for (i32 i = 0; i < this->myLength; ++i)
			this->myData[i].~T();
		this->myLength = 0;
	}

	FORCEINLINE void Resize(const i32 aNewSize)
	{
		if (aNewSize == this->GetLength())
			return;

		Reallocate(aNewSize);

		for (i32 i = this->myLength; i < aNewSize; ++i)
			new (&this->myData[i]) T();

		this->myLength = aNewSize;

		if constexpr (ZeroOnePastEnd)
			memset(&this->myData[this->myLength], 0, sizeof T);
	}

	FORCEINLINE void PrepareAdd(const i32 aElementCount)
	{
		if (aElementCount <= 0)
			return;
		MakeSizeFor(this->GetLength() + aElementCount);
	}

	FORCEINLINE void SetMinCapacity(const i32 aCapacity)
	{
		const i32 CapacityTarget = CeilToPowerTwo(aCapacity);
		if (CapacityTarget > GetCapacity())
			Reallocate(CapacityTarget);
	}


	FORCEINLINE void SetMinLength(const i32 aLength)
	{
		if (aLength > this->GetLength())
			Resize(aLength);
	}

	FORCEINLINE T Pop()
	{
		if (this->GetLength() == 0)
			abort();

		--this->myLength;
		T val = Move(this->myData[this->myLength]);
		this->myData[this->myLength].~T();
		return val;
	}

	FORCEINLINE bool TryPop()
	{
		if (this->GetLength() == 0)
			return false;

		Pop();
		return true;
	}
	
	FORCEINLINE bool TryPop(T& aElement)
	{
		if (this->GetLength() == 0)
			return false;

		--this->myLength;
		aElement = Move(this->myData[this->myLength]);
		this->myData[this->myLength].~T();
		return true;
	}

	i32 ShaveFromStart(const T & aValue)
	{
		i32 removedElements = 0;
		while (removedElements < this->GetLength())
		{
			if (this->myData[removedElements] != aValue)
				break;
			++removedElements;
		}

		RemoveRange(0, removedElements);

		return removedElements;
	}

	i32 ShaveFromEnd(const T & aValue)
	{
		i32 removedElements = 0;
		while (removedElements < this->GetLength() && this->myData[this->myLength - removedElements - 1] == aValue)
			++removedElements;
		RemoveRange(this->myLength - removedElements, removedElements);
		return removedElements;
	}

	i32 RemoveRange(const i32 aStart, const i32 aLength)
	{
		// TODO: Optimize
		for (i32 i = 0; i < aLength; ++i)
			RemoveAt(aStart);

		return aLength;

		// Non-working optimization attempt
		// if (aLength == 0)
		// 	return 0;
		// 
		// if (aLength < 0)
		// 	abort();
		// 
		// if (aStart < 0)
		// 	abort();
		// 
		// if (aStart + aLength > this->GetLength())
		// 	abort();
		// 
		// for (i32 i = 0; i < aLength && aStart + aLength + i < this->myLength; ++i)
		// 	this->myData[aStart + i] = Move(this->myData[aStart + aLength + i]);
		// 
		// for (i32 i = aStart + aLength; i < this->myLength; ++i)
		// 	this->myData[i].~T();
		// 
		// this->myLength -= aLength;
		// return aLength;
	}

private:
	FORCEINLINE void ClearAndSetCapacity(const i32 aNewCapacity)
	{
		Clear();
		Reallocate(aNewCapacity);
	}

	FORCEINLINE void MakeSizeFor(const i32 aNewCapacity)
	{
		if (aNewCapacity > myCapacity)
			Reallocate(aNewCapacity);
	}

	void Reallocate(const i32 aNewCapacity)
	{
		T* oldData = this->myData;

		i32 allocationSize = aNewCapacity;
		if constexpr (ZeroOnePastEnd)
			++allocationSize;

		allocationSize = CeilToPowerTwo(allocationSize);
		this->myData = static_cast<T*>(this->AllocateRaw(allocationSize, myCapacity));

		if (!this->myData)
			abort();

		if constexpr (ZeroOnePastEnd)
			--myCapacity;

		const i32 elementsToKeep = Min(this->myLength, myCapacity);

		for (i32 i = 0; i < elementsToKeep; ++i)
			new (static_cast<void*>(&this->myData[i])) T(Move(oldData[i]));

		for (i32 i = 0; i < this->myLength; ++i)
			oldData[i].~T();

		if (myCapacity < this->myLength)
			this->myLength = myCapacity;

		this->FreeRaw(oldData);

		if constexpr (ZeroOnePastEnd)
			memset(&this->myData[this->myLength], 0, sizeof T);
	}

	i32 myCapacity = 0;
};
