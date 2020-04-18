#pragma once

template <typename T, typename TAllocator, bool ZeroOnePastEnd>
class Array;
template <typename T>
class ArraySlice;

template <typename T>
class ArrayView
{
public:
	using iterator = T *;
	using const_iterator = const T*;

	FORCEINLINE iterator begin()
	{
		return myData;
	}

	FORCEINLINE const_iterator begin() const
	{
		return myData;
	}

	FORCEINLINE iterator end()
	{
		return &myData[myLength];
	}

	FORCEINLINE const_iterator end() const
	{
		return &myData[myLength];
	}

	FORCEINLINE ArrayView() = default;

	FORCEINLINE ArrayView(const T* aArray, const i32 aLength)
	{
		this->myData = const_cast<T*>(aArray);
		this->myLength = aLength;
	}

	FORCEINLINE ArrayView(std::initializer_list<T> aData)
	{
		this->myData = const_cast<T*>(aData.begin());
		this->myLength = static_cast<i32>(aData.size());
	}

	template <i32 TLength>
	FORCEINLINE ArrayView(const T (&aArray)[TLength])
	{
		this->myData = const_cast<T*>(aArray);
		// TODO high-prio: For some reason String/StringView construction from string literals take this path, so this is needed here but is unwanted behavior otherwise as it will truncate the end of the array
		this->myLength = TLength == 0 ? 0 : TLength - 1;
	}

	FORCEINLINE ArrayView(const ArrayView& aCopy)
	{
		*this = aCopy;
	}

	FORCEINLINE ~ArrayView() = default;

	template <typename TAllocator, bool ZeroOnePastEnd>
	ArrayView(Array<T, TAllocator, ZeroOnePastEnd>&&) = delete;
	template <typename TAllocator, bool ZeroOnePastEnd>
	ArrayView& operator=(Array<T, TAllocator, ZeroOnePastEnd>&&) = delete;

	FORCEINLINE ArrayView& operator=(const ArrayView& aCopy)
	{
		this->myData = aCopy.myData;
		this->myLength = aCopy.myLength;
		return *this;
	}

	FORCEINLINE const T* GetData() const
	{
		return this->myData;
	}

	FORCEINLINE const T& GetClamped(const i32 aIndex)
	{
#if ARRAY_BOUNDSCHECK
		if (myLength == 0)
			OutOfBounds();
#endif
		return operator[](Clamp(aIndex, 0, myLength - 1));
	}

	FORCEINLINE const T& GetClamped(const i32 aIndex) const
	{
#if ARRAY_BOUNDSCHECK
		if (myLength == 0)
			OutOfBounds();
#endif
		return operator[](Clamp(aIndex, 0, myLength));
	}

	FORCEINLINE T* TryGet(const i32 aIndex)
	{
		if (aIndex < 0 || aIndex >= this->myLength)
			return nullptr;
		return &this->myData[aIndex];
	}

	FORCEINLINE const T* TryGet(const i32 aIndex) const
	{
		if (aIndex < 0 || aIndex >= this->myLength)
			return nullptr;
		return &this->myData[aIndex];
	}

	FORCEINLINE T TryGet(const i32 aIndex, const T aDefaultValue) const
	{
		if (aIndex < 0 || aIndex >= this->myLength)
			return aDefaultValue;
		return this->myData[aIndex];
	}

	FORCEINLINE T GetOrDefault(const i32 aIndex) const
	{
		return TryGet(aIndex, T());
	}

	FORCEINLINE const T& operator[](const i32 aIndex) const
	{
#if ARRAY_BOUNDSCHECK
		if (aIndex < 0 || aIndex >= this->myLength)
			OutOfBounds();
#endif
		return this->myData[aIndex];
	}

	FORCEINLINE const T& First() const
	{
#if ARRAY_BOUNDSCHECK
		if (this->myLength == 0)
			OutOfBounds();
#endif
		return this->myData[0];
	}

	FORCEINLINE const T& Last() const
	{
#if ARRAY_BOUNDSCHECK
		if (this->myLength == 0)
			OutOfBounds();
#endif
		return this->myData[this->myLength - 1];
	}

	FORCEINLINE i32 GetLength() const
	{
		return this->myLength;
	}

	template <typename TComparer = LessThanEquals>
	bool IsSorted() const
	{
		if (this->myLength == 0)
			return true;

		for (i32 i = 0; i < this->myLength - 1; ++i)
		{
			if (TComparer::Compare(this->myData[i], this->myData[i + 1]))
				return false;
		}

		return true;
	}

	FORCEINLINE ArrayView Slice(const i32 aBegin, const i32 aLength) const
	{
#if ARRAY_BOUNDSCHECK
		if (aBegin < 0 || aLength < 0)
			OutOfBounds();
		if (aBegin + aLength > GetLength())
			OutOfBounds();
#endif
		return ArrayView(&myData[aBegin], aLength);
	}

	FORCEINLINE ArrayView SliceFromStart(const i32 aElements) const
	{
		return Slice(0, aElements);
	}

	FORCEINLINE ArrayView SliceFromEnd(const i32 aElements) const
	{
		return Slice(myLength - aElements, aElements);
	}

	FORCEINLINE ArrayView ClampedSlice(i32 aBegin, i32 aLength) const
	{
		i32 start = Clamp(aBegin, 0, myLength);
		i32 end = Clamp(aBegin + aLength, 0, myLength);
		return Slice(start, end - start);
	}

	FORCEINLINE ArrayView ClampedSliceFromStart(const i32 aElements) const
	{
		return ClampedSlice(0, aElements);
	}

	FORCEINLINE ArrayView ClampedSliceFromEnd(const i32 aElements) const
	{
		return ClampedSlice(myLength - aElements, aElements);
	}

	/** Returns the part at the end, use SliceFromStart to return from the start instead. */
	FORCEINLINE ArrayView RemoveFromStart(const i32 aElements) const
	{
		return Slice(aElements, this->myLength - aElements);
	}

	/** Returns the part at the start, use SliceFromEnd to return from the start instead. */
	FORCEINLINE ArrayView RemoveFromEnd(const i32 aElements) const
	{
		return Slice(0, this->myLength - aElements);
	}

	FORCEINLINE bool IsValidIndex(const i32 aIndex) const
	{
		return aIndex >= 0 && aIndex < myLength;
	}

	i32 Find(const T& aElement) const
	{
		for (i32 i = 0; i < myLength; ++i)
		{
			if (myData[i] == aElement)
				return i;
		}

		return -1;
	}

	i32 Compare(const ArrayView& aOther) const
	{
		if (aOther.GetLength() > this->GetLength())
			return -1;
		if (aOther.GetLength() < this->GetLength())
			return 1;

		for (i32 i = 0; i < myLength; ++i)
		{
			if (aOther[i] > myData[i])
				return -1;
			if (aOther[i] < myData[i])
				return 1;
		}

		return 0;
	}
	
	template <typename TComparer = LessThan>
	i32 FindLowerBoundIndex(const T& aOther) const
	{
		const i32 length = GetLength();

		i32 first = 0;

		while (length > 0)
		{
			const i32 step = length / 2;
			const i32 index = first + step;

			if (!TComparer::Compare(myData[index], aOther))
			{
				first = index + 1;
				length -= step + 1;
			}
			else
			{
				length = step;
			}
		}

		return first;
	}

	template <typename TComparer = LessThan>
	FORCEINLINE const T& FindLowerBoundRef(const T& aOther) const
	{
		return (*this)[FindLowerBoundIndex<TComparer>(aOther)]; 
	}

	template <typename TComparer = LessThan>
	i32 FindUpperBoundIndex(const T& aOther) const
	{
		const i32 length = GetLength();

		i32 first = 0;

		while (length > 0)
		{
			const i32 step = length / 2;
			const i32 index = first + step;

			if (!TComparer::Compare(aOther, myData[index]))
			{
				first = index + 1;
				length -= step + 1;
			}
			else
			{
				length = step;
			}
		}

		return first;
	}

	template <typename TComparer = LessThan>
	FORCEINLINE const T& FindUpperBoundRef(const T& aOther) const
	{
		return (*this)[FindUperBoundIndex<TComparer>(aOther)];
	}

	bool operator>=(const ArrayView& aOther) const { return Compare(aOther) >= 0; }
	bool operator>(const ArrayView& aOther) const { return Compare(aOther) > 0; }
	bool operator<=(const ArrayView& aOther) const { return Compare(aOther) <= 0; }
	bool operator<(const ArrayView& aOther) const { return Compare(aOther) < 0; }

	bool operator==(const ArrayView& aString) const
	{
		if (myLength != aString.myLength)
			return false;

		for (i32 i = 0; i < myLength; ++i)
		{
			if (myData[i] != aString.myData[i])
				return false;
		}

		return true;
	}

	bool operator!=(const ArrayView& aString) const
	{
		return !(*this == aString);
	}

	FORCEINLINE ArrayView View() const
	{
		return *this;
	}

	bool StartsWith(const ArrayView& aArray) const
	{
		if (GetLength() < aArray.GetLength())
			return false;

		return SliceFromStart(aArray.GetLength()) == aArray;
	}

	bool EndsWith(const ArrayView& aArray) const
	{
		if (GetLength() < aArray.GetLength())
			return false;

		return SliceFromEnd(aArray.GetLength()) == aArray;
	}

protected:
#if ARRAY_BOUNDSCHECK
	static void OutOfBounds()
	{
		abort();
	}
#endif

	T* myData = nullptr;
	i32 myLength = 0;
};

template <typename T>
FORCEINLINE ::std::wostream& operator<<(::std::wostream& aOut, const ArrayView<T>& aArray)
{
	aOut << typeid(T).name() << L"[" << aArray.GetLength() << L"] { ";

	for (i32 i = 0; i < aArray.GetLength(); ++i)
	{
		aOut << aArray[i];

		if (i + 1 != aArray.GetLength())
			aOut << L", ";
		else
			aOut << L" ";
	}
	aOut << L"}";

	return aOut;
}

#define DECLARE_ARRAY_SLICE_FUNCTIONS(ClassName) \
	struct UpcastType { }; \
	FORCEINLINE ClassName Slice(const i32 aBegin, const i32 aLength) const { return ClassName(Super::Slice(aBegin, aLength), ClassName::UpcastType()); } \
	FORCEINLINE ClassName SliceFromStart(const i32 aElements) const { return ClassName(Super::SliceFromStart(aElements), ClassName::UpcastType()); } \
	FORCEINLINE ClassName SliceFromEnd(const i32 aElements) const { return ClassName(Super::SliceFromEnd(aElements), ClassName::UpcastType()); } \
	FORCEINLINE ClassName ClampedSlice(const i32 aBegin, const i32 aLength) const { return ClassName(Super::ClampedSlice(aBegin, aLength), ClassName::UpcastType()); } \
	FORCEINLINE ClassName ClampedSliceFromStart(const i32 aElements) const { return ClassName(Super::ClampedSliceFromStart(aElements), ClassName::UpcastType()); } \
	FORCEINLINE ClassName ClampedSliceFromEnd(const i32 aElements) const { return ClassName(Super::ClampedSliceFromEnd(aElements), ClassName::UpcastType()); } \
	FORCEINLINE ClassName RemoveFromStart(const i32 aElements) const { return ClassName(Super::RemoveFromStart(aElements), ClassName::UpcastType()); } \
	FORCEINLINE ClassName RemoveFromEnd(const i32 aElements) const { return ClassName(Super::RemoveFromEnd(aElements), ClassName::UpcastType()); }
