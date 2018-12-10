#pragma once

template <typename T>
class FactoryChunk
{
public:
	FactoryChunk(const u16 aSize)
	{
		ASSUME(aSize >= 4);

		myObjectsData = static_cast<T*>(_aligned_malloc(sizeof(T) * aSize, alignof(T)));
		CHECK(myObjectsData != nullptr);
		mySize = aSize;
		myAllocatedObjects = 0;
		myMaxOccupiedIndex = 0;

		myOccupiedSlots.Resize(mySize);

		for (u16 i = 0; i < aSize; ++i)
			myOccupiedSlots[i] = false;
	}

	virtual ~FactoryChunk()
	{
		if (!IsEmpty())
			FATAL(L"References to this factory chunk remains!");

		_aligned_free(myObjectsData);
		mySize = 0;
	}

	FORCEINLINE u16 GetSize() const
	{
		return mySize;
	}

	template <typename TCallback>
	void ForEach(const TCallback& callback) const
	{
		if (IsEmpty())
			return;

		if (!IsFull())
		{
			for (u16 i = 0; i < mySize; ++i)
			{
				if (myOccupiedSlots[i])
					callback(myObjectsData[i]);
			}
		}
		else
		{
			for (u16 i = 0; i < mySize; ++i)
				callback(myObjectsData[i]);
		}
	}

	virtual T& Allocate()
	{
		if (IsFull())
			FATAL(L"This factory chunk has no free room");

		++myAllocatedObjects;

		for (u16 i = 0; i < mySize; ++i)
		{
			if (myOccupiedSlots[i] == false)
			{
				myOccupiedSlots[i] = true;

				if (i + 1 > myMaxOccupiedIndex)
					myMaxOccupiedIndex = i + 1;

				T& object = myObjectsData[i];
				new(static_cast<void*>(&object)) T();
				return object;
			}
		}

		// Should be unreachable except if we made a threading error
		abort();
	}

	FORCEINLINE bool IsEmpty() const
	{
		return myAllocatedObjects == 0;
	}

	FORCEINLINE bool IsFull() const
	{
		return myAllocatedObjects >= mySize;
	}

	virtual void Return(const T& aObject)
	{
		const u16 index = IndexOf(aObject);

		aObject.~T();
		memset(&const_cast<T&>(aObject), 0, sizeof T);

		// Object was returned twice or was never allocated
		CHECK(myOccupiedSlots[index]);

		myOccupiedSlots[index] = false;

		--myAllocatedObjects;

		while (myMaxOccupiedIndex > 0 && !myOccupiedSlots[myMaxOccupiedIndex - 1])
			--myMaxOccupiedIndex;
	}

	FORCEINLINE bool DoesObjectOriginateFromHere(const T& aObject) const
	{
		return (&aObject >= myObjectsData) && (&aObject < myObjectsData + mySize);
	}

protected:
	FORCEINLINE u16 IndexOf(const T& aObject) const
	{
		CHECK(DoesObjectOriginateFromHere(aObject));
		return static_cast<u16>(&aObject - myObjectsData);
	}

	template <typename TCallback>
	FORCEINLINE void IteratePotentialIndices(const TCallback aCallback)
	{
		if (IsEmpty())
			return;

		for (u16 i = 0; i < myMaxOccupiedIndex; ++i)
			aCallback(i);
	}

	FORCEINLINE const T* GetData() const
	{
		return myObjectsData;
	}

private:
	Array<bool> myOccupiedSlots;
	T* myObjectsData;
	u16 mySize;
	u16 myAllocatedObjects;
	
	u16 myMaxOccupiedIndex;
};
