#pragma once
#include "Hashing.h"

template <typename T>
class MapIterator
{
public:
	using iterator = MapIterator;
	using iterator_category = std::forward_iterator_tag;
	using value_type = typename RemoveConst<T>;
	using difference_type = ptrdiff_t;
	using pointer = T * ;
	using reference = T & ;

	FORCEINLINE MapIterator(Array<Array<T>>* aBuckets)
	{
		myBuckets = aBuckets;
		while (myBucketIndex < myBuckets->GetLength() && (*myBuckets)[myBucketIndex].GetLength() == 0)
			++myBucketIndex;
	}

	FORCEINLINE MapIterator(Array<Array<T>>* aBuckets, i32 aBucketIndex, i32 aValueIndex)
	{
		myBuckets = aBuckets;
		myBucketIndex = aBucketIndex;
		myValueIndex = aValueIndex;
		while (myBucketIndex < myBuckets->GetLength() && (*myBuckets)[myBucketIndex].GetLength() == 0)
		{
			++myBucketIndex;
			myValueIndex = 0;
		}
	}

	FORCEINLINE iterator& operator=(Array<Array<T>>* aBuckets)
	{
		myBuckets = &aBuckets;
		myBucketIndex = 0;
		myValueIndex = 0;
		return *this;
	}

	FORCEINLINE MapIterator(const iterator&) = default;
	FORCEINLINE iterator& operator=(const iterator&) = default;

	FORCEINLINE iterator& operator++()
	{
		++myValueIndex;
		if (myValueIndex >= (*myBuckets)[myBucketIndex].GetLength())
		{
			myValueIndex = 0;

			++myBucketIndex;
			while (myBucketIndex < myBuckets->GetLength() && (*myBuckets)[myBucketIndex].GetLength() == 0)
				++myBucketIndex;
		}
		return *this;
	}

	FORCEINLINE iterator operator++(int) // postfix increment
	{
		iterator before = *this;
		*this++;
		return before;
	}

	FORCEINLINE pointer operator->() const
	{
		return &((*myBuckets)[myBucketIndex][myValueIndex]);
	}

	FORCEINLINE reference operator*() const
	{
		return (*myBuckets)[myBucketIndex][myValueIndex];
	}

	FORCEINLINE friend bool operator==(const iterator & aLeft, const iterator & aRight)
	{
		return aLeft.myBuckets == aRight.myBuckets &&
			aLeft.myBucketIndex == aRight.myBucketIndex &&
			aLeft.myValueIndex == aRight.myValueIndex;
	}

	FORCEINLINE friend bool operator!=(const iterator & aLeft, const iterator & aRight)
	{
		return !(aLeft == aRight);
	}

private:
	Array<Array<T>> * myBuckets = nullptr;
	i32 myBucketIndex = 0;
	i32 myValueIndex = 0;
};

template <typename T>
class ConstMapIterator
{
public:
	using iterator = ConstMapIterator;
	using iterator_category = std::forward_iterator_tag;
	using value_type = T;
	using difference_type = ptrdiff_t;
	using pointer = const T*;
	using reference = const T&;

	FORCEINLINE ConstMapIterator(const Array<Array<T>>* aBuckets)
	{
		myBuckets = aBuckets;
		while (myBucketIndex < myBuckets->GetLength() && (*myBuckets)[myBucketIndex].GetLength() == 0)
			++myBucketIndex;
	}

	FORCEINLINE ConstMapIterator(const Array<Array<T>>* aBuckets, i32 aBucketIndex, i32 aValueIndex)
	{
		myBuckets = aBuckets;
		myBucketIndex = aBucketIndex;
		myValueIndex = aValueIndex;
		while (myBucketIndex < myBuckets->GetLength() && (*myBuckets)[myBucketIndex].GetLength() == 0)
		{
			++myBucketIndex;
			myValueIndex = 0;
		}
	}

	FORCEINLINE iterator& operator=(const Array<Array<T>>* aBuckets)
	{
		myBuckets = &aBuckets;
		myBucketIndex = 0;
		myValueIndex = 0;
		return *this;
	}

	FORCEINLINE ConstMapIterator(const iterator&) = default;
	FORCEINLINE iterator& operator=(const iterator&) = default;

	FORCEINLINE iterator& operator++()
	{
		++myValueIndex;
		if (myValueIndex >= (*myBuckets)[myBucketIndex].GetLength())
		{
			myValueIndex = 0;

			++myBucketIndex;
			while (myBucketIndex < myBuckets->GetLength() && (*myBuckets)[myBucketIndex].GetLength() == 0)
				++myBucketIndex;
		}
		return *this;
	}

	FORCEINLINE iterator operator++(int) // postfix increment
	{
		iterator before = *this;
		*this++;
		return before;
	}

	FORCEINLINE pointer operator->() const
	{
		return &((*myBuckets)[myBucketIndex][myValueIndex]);
	}

	FORCEINLINE reference operator*() const
	{
		return (*myBuckets)[myBucketIndex][myValueIndex];
	}

	FORCEINLINE friend bool operator==(const iterator & aLeft, const iterator & aRight)
	{
		return aLeft.myBuckets == aRight.myBuckets &&
			aLeft.myBucketIndex == aRight.myBucketIndex &&
			aLeft.myValueIndex == aRight.myValueIndex;
	}

	FORCEINLINE friend bool operator!=(const iterator & aLeft, const iterator & aRight)
	{
		return !(aLeft == aRight);
	}

private:
	const Array<Array<T>>* myBuckets = nullptr;
	i32 myBucketIndex = 0;
	i32 myValueIndex = 0;
};

template <typename TKey, typename TValue>
struct KeyValuePair
{
	TKey key;
	TValue value;
};

class StringKey
{
public:
	StringKey(StringView aString)
	{
		myView = aString;
	}
	StringKey(String&& aString)
	{
		myView = aString.View();
	}

	operator StringView&()
	{
		return myView;
	}

	operator const StringView&() const
	{
		return myView;
	}

	bool operator==(const StringView& aOther) const
	{
		return myView == aOther;
	}

	bool operator!=(const StringView& aOther) const
	{
		return myView != aOther;
	}

private:
	StringView myView;
};

template<>
FORCEINLINE size_t HashOf<StringKey>(const StringKey& aValue)
{
	return HashOf<StringView>(aValue);
}

template <typename TKey, typename TValue>
class Map
{
	using FindType = typename Select<IsSame<TKey, String>, StringKey, TKey>;
	using Pair = KeyValuePair<TKey, TValue>;

public:
	using iterator = MapIterator<Pair>;
	using const_iterator = ConstMapIterator<Pair>;

	iterator begin() { return iterator(&myBuckets); }
	const_iterator begin() const { return const_iterator(&myBuckets); }

	iterator end() { return iterator(&myBuckets, myBuckets.GetLength(), 0); }
	const_iterator end() const { return const_iterator(&myBuckets, myBuckets.GetLength(), 0); }

	FORCEINLINE Map() = default;
	FORCEINLINE ~Map() = default;
	FORCEINLINE Map(const Map& aCopy) = default;
	FORCEINLINE Map& operator=(const Map& aCopy) = default;

	FORCEINLINE Map(Map&& aMove) noexcept
	{
		myBuckets = Move(aMove.myBuckets);
	}

	FORCEINLINE Map& operator=(Map&& aMove) noexcept
	{
		myBuckets = Move(aMove.myBuckets);
		return *this;
	}

	FORCEINLINE i32 GetLength() const
	{
		return myLength;
	}

	FORCEINLINE const TValue* Find(const FindType& aKey) const
	{
		return const_cast<Map*>(this)->Find(aKey);
	}

	FORCEINLINE TValue Find(const FindType& aKey, const TValue& aDefault) const
	{
		TValue* value = const_cast<Map*>(this)->Find(aKey);
		if (!value)
			return aDefault;
		return *value;
	}

	FORCEINLINE TValue* Find(const FindType& aKey)
	{
		if (myBuckets.GetLength() == 0)
			return nullptr;

		const size_t hash = HashOf<FindType>(aKey);
		const i32 bucketIndex = hash % myBuckets.GetLength();
		Array<Pair>& values = myBuckets[bucketIndex];
		for (i32 i = 0; i < values.GetLength(); ++i)
		{
			if (aKey == values[i].key)
				return &values[i].value;
		}
		return nullptr;
	}

	FORCEINLINE TValue& FindOrAdd(FindType aKey)
	{
		ConsiderGrow();

		const size_t hash = HashOf<FindType>(aKey);
		const i32 bucketIndex = hash % myBuckets.GetLength();
		Array<Pair>& values = myBuckets[bucketIndex];
		for (i32 i = 0; i < values.GetLength(); ++i)
		{
			if (aKey == values[i].key)
				return values[i].value;
		}

		++myLength;
		Pair& pair = values.Add(Pair());
		pair.key = TKey(Move(aKey));
		pair.value = TValue();
		return pair.value;
	}

	FORCEINLINE TValue& Add(TKey aKey, TValue aValue)
	{
		ConsiderGrow();

		const size_t hash = HashOf<TKey>(aKey);
		const i32 bucketIndex = hash % myBuckets.GetLength();
		Array<Pair>& values = myBuckets[bucketIndex];
		for (i32 i = 0; i < values.GetLength(); ++i)
		{
			if (aKey == values[i].key)
				FATAL(L"Key already present");
		}

		++myLength;
		Pair& pair = values.Add(Pair());
		pair.key = Move(aKey);
		pair.value = TValue(Move(aValue));
		return pair.value;
	}

	FORCEINLINE TValue& operator[](FindType aKey)
	{
		return FindOrAdd(aKey);
	}

	FORCEINLINE const TValue& operator[](FindType aKey) const
	{
		const TValue* value = Find(aKey);
		CHECK(value);
		return *value;
	}

	FORCEINLINE bool ContainsKey(const FindType& aKey) const
	{
		return Find(aKey) != nullptr;
	}

	FORCEINLINE bool Remove(const FindType& aKey)
	{
		if (myBuckets.GetLength() == 0)
			return false;

		const size_t hash = HashOf<FindType>(aKey);
		const i32 bucketIndex = hash % myBuckets.GetLength();
		Array<Pair>& values = myBuckets[bucketIndex];
		for (i32 i = 0; i < values.GetLength(); ++i)
		{
			if (aKey == values[i].key)
			{
				--myLength;
				values.RemoveAtSwap(i);
				return true;
			}
		}
		return false;
	}

	void Clear()
	{
		myBuckets.Clear();
		myLength = 0;
	}

	template <typename TCallback>
	void ForEach(const TCallback& aCallback)
	{
		for (auto& pairList : myBuckets)
		{
			for (auto& pair : pairList)
				aCallback(pair);
		}
	}

	template <typename TCallback>
	void ForEach(const TCallback& aCallback) const
	{
		for (const auto& pairList : myBuckets)
		{
			for (const auto& pair : pairList)
				aCallback(pair);
		}
	}

private:
	FORCEINLINE void Grow(const i32 aNewBucketSize)
	{
		Map newMap;
		newMap.myBuckets.Resize(aNewBucketSize);

		for (i32 iBucket = 0; iBucket < myBuckets.GetLength(); ++iBucket)
		{
			Array<Pair>& values = myBuckets[iBucket];
			for (i32 iValue = 0; iValue < values.GetLength(); ++iValue)
			{
				Pair& pair = values[iValue];
				newMap.Add(Move(pair.key), Move(pair.value));
			}
		}

		*this = Move(newMap);
	}

	FORCEINLINE void ConsiderGrow()
	{
		if (myBuckets.GetLength() == 0)
		{
			Grow(16);
		}
		else
		{
			if (myLength > (myBuckets.GetLength() >> 1))
				Grow(myBuckets.GetLength() * 2);
		}
	}

	Array<Array<Pair>> myBuckets;
	i32 myLength = 0;
};
