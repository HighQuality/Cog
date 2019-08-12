#pragma once
#include "ArrayView.h"

template <typename T, typename TAllocator, bool ZeroOnePastEnd>
class Array;

template <typename T>
class ArraySlice : public ArrayView<T>
{
public:
	using Super = ArrayView<T>;

	FORCEINLINE ArraySlice() = default;
	FORCEINLINE ~ArraySlice() = default;
	FORCEINLINE ArraySlice(const ArraySlice&) = default;
	FORCEINLINE ArraySlice& operator=(const ArraySlice&) = default;
	FORCEINLINE ArraySlice(ArraySlice&&) noexcept = default;
	FORCEINLINE ArraySlice& operator=(ArraySlice&&) noexcept = default;

	template <typename TAllocator, bool ZeroOnePastEnd>
	ArraySlice(Array<T, TAllocator, ZeroOnePastEnd>&&) = delete;
	template <typename TAllocator, bool ZeroOnePastEnd>
	ArraySlice& operator=(Array<T, TAllocator, ZeroOnePastEnd>&&) = delete;

	FORCEINLINE T& operator[](const i32 aIndex)
	{
#if ARRAY_BOUNDSCHECK
		if (aIndex < 0 || aIndex >= this->myLength)
			this->OutOfBounds();
#endif
		return this->myData[aIndex];
	}

	FORCEINLINE const T& operator[](const i32 aIndex) const
	{
#if ARRAY_BOUNDSCHECK
		if (aIndex < 0 || aIndex >= this->myLength)
			this->OutOfBounds();
#endif
		return this->myData[aIndex];
	}

	FORCEINLINE T& First() const
	{
#if ARRAY_BOUNDSCHECK
		if (this->myLength == 0)
			this->OutOfBounds();
#endif
		return this->myData[0];
	}

	FORCEINLINE T& Last() const
	{
#if ARRAY_BOUNDSCHECK
		if (this->myLength == 0)
			this->OutOfBounds();
#endif
		return this->myData[this->myLength - 1];
	}

	FORCEINLINE T* GetData() const
	{
		return this->myData;
	}

	inline void Reverse()
	{
		const i32 length = this->GetLength();

		i32 b = 0;
		i32 e = length;

		while (b != e && b != --e)
		{
			Swap(this->myData[b], this->myData[e]);
			++b;
		}
	}

	void Shuffle()
	{
		if (this->myLength <= 1)
			return;

		for (i32 i = 0; i < this->myLength - 1; ++i)
			Swap(this->myData[i], this->myData[RandInteger(i, this->myLength)]);
	}

	inline void QuickSort()
	{
		QuickSort(&ArraySlice::LessThan);
	}

	inline void QuickSortReverse()
	{
		QuickSort(&ArraySlice::GreaterThan);
	}

	template <typename TComparer>
	inline void QuickSort(const TComparer& aComparer)
	{
		if (this->GetLength() <= 1)
			return;
		
		T* data = this->myData;
		const i32 length = this->myLength;

		const i32 randomIndex = RandInteger(0, length);
		const i32 pivotIndex = length - 1;
		if (randomIndex != pivotIndex)
			Swap(data[randomIndex], data[pivotIndex]);
		const T& pivotValue = data[pivotIndex];

		i32 smallIndex = -1;

		for (i32 i = 0; i < length - 1; ++i)
		{
			const T& value = data[i];
			
			if (value == pivotValue || aComparer(value, pivotValue))
			{
				++smallIndex;
				Swap(data[i], data[smallIndex]);
			}
		}

		Swap(data[smallIndex + 1], data[pivotIndex]);

		SliceFromStart(smallIndex + 1).QuickSort(aComparer);
		SliceFromEnd(length - (smallIndex + 2)).QuickSort(aComparer);
	}

	DECLARE_ARRAY_SLICE_FUNCTIONS(ArraySlice)

	FORCEINLINE explicit ArraySlice(ArrayView<T>&& aBase, UpcastType&&)
		: Super(Move(aBase))
	{
	}

private:
	static bool LessThan(const T& aLeft, const T& aRight)
	{
		return aLeft < aRight;
	}

	static bool GreaterThan(const T& aLeft, const T& aRight)
	{
		return aLeft > aRight;
	}
};
