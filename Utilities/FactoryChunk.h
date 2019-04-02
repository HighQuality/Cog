#pragma once

enum class FactoryChunkSlotStatus : u8
{
	Free,
	Occupied,
	Deactivated
};

class ChunkedData
{
public:
	template <typename ...TArgs>
	ChunkedData(u16 aNumObjects, TArgs& ...aArgs)
	{
		myNumObjects = aNumObjects;
		
		const Array<i32> sizePerVariable { sizeof(*aArgs)... };

		i32 sizePerObject = 0;
		for (i32 size : sizePerVariable)
			sizePerObject += size;

		i32 sizeBytes = aNumObjects * sizePerObject;
		// Add some padding for per-variable alignment
		sizeBytes += 32 * sizePerVariable.GetLength();

		if (sizeBytes > 0)
		{
			myData = static_cast<u8*>(malloc(sizeBytes));
			CHECK(myData);

			myDataEnd = myData + sizeBytes;

			(AllocateChunkData(aArgs), ...);
		}
	}

	virtual ~ChunkedData()
	{
		if (myData)
		{
			free(myData);
			myData = nullptr;
		}
	}
	
	virtual void DefaultInitializeIndex(const u16 aIndex)
	{ }

	virtual void DestructIndex(const u16 aIndex)
	{ }

protected:
	FORCEINLINE u16 GetNumObjects() const { return myNumObjects; }

private:
	template <typename T>
	void AllocateChunkData(T*& aData)
	{
		const i32 sizeBytes = sizeof(T) * myNumObjects;

		CHECK(&myData[myDataPointer + sizeBytes] <= myDataEnd);

		T* data = reinterpret_cast<T*>(&myData[myDataPointer]);
		myDataPointer += sizeBytes;

		// Align next allocation to 32 bytes
		const u16 mod = myDataPointer % 32;
		if (mod > 0)
			myDataPointer += 32 - mod;

		aData = data;
	}

	u8* myData = nullptr;
	u8* myDataEnd = nullptr;
	i32 myDataPointer = 0;
	u16 myNumObjects = 0;
};

#define DECLARE_CHUNKED_DATA(TChunkData) \
	public: \
	using ChunkData = TChunkData; \
	private: \
	friend class TChunkData; \
	FORCEINLINE ChunkData* GetChunkedData() const { return static_cast<ChunkData*>(myChunk->GetChunkedData()); }

#define DECLARE_CHUNKED_PROPERTY_ACCESSORS(PropertyName, SetterVisibility, GetterVisibility) \
	SetterVisibility: FORCEINLINE void Set##PropertyName(RemoveReference<decltype(ChunkData::my##PropertyName[0])> aNewValue) { GetChunkedData()->my##PropertyName[myChunkIndex] = Move(aNewValue); }\
	GetterVisibility: FORCEINLINE const RemoveReference<decltype(ChunkData::my##PropertyName[0])>& Get##PropertyName() const { return GetChunkedData()->my##PropertyName[myChunkIndex]; }\
	private:

class BaseFactoryChunk
{
public:
	BaseFactoryChunk(const u16 aNumObjects)
	{
		CHECK(aNumObjects >= 4);

		myNumObjects = aNumObjects;
		myAllocatedObjects = 0;
		myMaxOccupiedIndex = 0;

		mySlotsStatus.Resize(myNumObjects);
		myGeneration.Resize(myNumObjects);
		myIsPendingDestroy.Resize(myNumObjects);

		for (u16 i = 0; i < aNumObjects; ++i)
		{
			mySlotsStatus[i] = FactoryChunkSlotStatus::Free;
			myGeneration[i] = 1;
			myIsPendingDestroy[i] = true;
		}
	}

	virtual ~BaseFactoryChunk() = default;

	FORCEINLINE bool IsEmpty() const
	{
		return myAllocatedObjects == 0;
	}

	FORCEINLINE bool IsFull() const
	{
		return myAllocatedObjects >= myNumObjects;
	}

	FORCEINLINE bool IsAllObjectsActivated() const
	{
		return myDeactivatedObjects == 0;
	}

	bool IsActivated(const u16 aIndex) const
	{
		return mySlotsStatus[aIndex] == FactoryChunkSlotStatus::Occupied;
	}

	void SetActivated(const u16 aIndex, const bool aIsActivated)
	{
		CHECK(mySlotsStatus[aIndex] != FactoryChunkSlotStatus::Free);

		FactoryChunkSlotStatus& status = mySlotsStatus[aIndex];
		const FactoryChunkSlotStatus targetStatus = aIsActivated ? FactoryChunkSlotStatus::Occupied : FactoryChunkSlotStatus::Deactivated;

		if (targetStatus != status)
		{
			status = targetStatus;

			if (aIsActivated)
				myDeactivatedObjects.fetch_sub(1);
			else
				myDeactivatedObjects.fetch_add(1);
		}
	}

	FORCEINLINE u16 FindGeneration(const u16 aIndex) const
	{
		return myGeneration[aIndex];
	}

	FORCEINLINE bool IsPendingDestroy(const u16 aIndex) const
	{
		return myIsPendingDestroy[aIndex];
	}

	FORCEINLINE void MarkPendingDestroy(const u16 aIndex)
	{
		myIsPendingDestroy[aIndex] = true;
	}

	FORCEINLINE ChunkedData* GetChunkedData() const { return myChunkedData; }

	virtual void InitializeIndex(const u16 aIndex)
	{
		if (myChunkedData)
			myChunkedData->DefaultInitializeIndex(aIndex);

		++myGeneration[aIndex];
	}

	virtual void DestructIndex(const u16 aIndex)
	{
		if (myChunkedData)
			myChunkedData->DestructIndex(aIndex);

		++myGeneration[aIndex];
	}

	virtual void ReturnByIndex(u16 aIndex) = 0;

protected:
	Array<FactoryChunkSlotStatus> mySlotsStatus;
	Array<u16> myGeneration;
	Array<bool> myIsPendingDestroy;

	ChunkedData* myChunkedData = nullptr;

	std::atomic<u16> myDeactivatedObjects;

	u16 myNumObjects;
	u16 myAllocatedObjects;

	u16 myMaxOccupiedIndex;
};


template <typename T>
auto CreateChunkedData(const u16 aNumObjects, i32) -> typename T::ChunkData*
{
	return new typename T::ChunkData(aNumObjects);
}

template <typename T>
ChunkedData* CreateChunkedData(const u16, ...)
{
	return nullptr;
}

template <typename T>
class FactoryChunk : public BaseFactoryChunk
{
public:
	using Base = BaseFactoryChunk;
	using ChunkedDataType = RemovePointer<decltype(CreateChunkedData<T>(1, 0))>;

	FactoryChunk(const u16 aNumObjects)
		: Base(aNumObjects)
	{
		myChunkedData = CreateChunkedData<T>(aNumObjects, 0);

		myObjectsData = static_cast<T*>(_aligned_malloc(sizeof(T) * aNumObjects, alignof(T)));
		CHECK(myObjectsData != nullptr);
	}

	virtual ~FactoryChunk()
	{
		if (!IsEmpty())
			FATAL(L"References to this factory chunk remains!");

		delete myChunkedData;
		myChunkedData = nullptr;

		_aligned_free(myObjectsData);
		myNumObjects = 0;
	}

	FORCEINLINE u16 GetSize() const
	{
		return myNumObjects;
	}

	template <typename TCallback>
	void ForEach(const TCallback& callback) const
	{
		if (IsEmpty())
			return;

		if (!IsFull() || !IsAllObjectsActivated())
		{
			for (u16 i = 0; i < myMaxOccupiedIndex; ++i)
			{
				if (mySlotsStatus[i] == FactoryChunkSlotStatus::Occupied)
					callback(myObjectsData[i]);
			}
		}
		else
		{
			for (u16 i = 0; i < myNumObjects; ++i)
				callback(myObjectsData[i]);
		}
	}

	virtual T& Allocate()
	{
		if (IsFull())
			FATAL(L"This factory chunk has no free room");

		++myAllocatedObjects;

		for (u16 i = 0; i < myNumObjects; ++i)
		{
			if (mySlotsStatus[i] == FactoryChunkSlotStatus::Free)
			{
				mySlotsStatus[i] = FactoryChunkSlotStatus::Occupied;

				if (i + 1 > myMaxOccupiedIndex)
					myMaxOccupiedIndex = i + 1;

				Object& object = myObjectsData[i];

				InitializeIndex(i);

				new(static_cast<void*>(&object)) T();

				object.myChunk = this;
				object.myChunkIndex = i;

				return static_cast<T&>(object);
			}
		}

		// Should be unreachable except if we made a threading error
		abort();
	}

	void ReturnAll()
	{
		ForEach([this](const T& aObject)
		{
			Return(aObject);
		});
	}

	virtual void Return(const T& aObject)
	{
		const u16 index = IndexOf(aObject);

		const FactoryChunkSlotStatus previousSlotStatus = mySlotsStatus[index];

		// Entity was returned twice or was never allocated
		CHECK(previousSlotStatus != FactoryChunkSlotStatus::Free);

		DestructIndex(index);

		aObject.~T();
		memset(&const_cast<T&>(aObject), 0, sizeof T);

		if (previousSlotStatus == FactoryChunkSlotStatus::Deactivated)
			myDeactivatedObjects.fetch_sub(1);

		mySlotsStatus[index] = FactoryChunkSlotStatus::Free;

		--myAllocatedObjects;

		while (myMaxOccupiedIndex > 0 && mySlotsStatus[myMaxOccupiedIndex - 1] == FactoryChunkSlotStatus::Free)
			--myMaxOccupiedIndex;
	}

	FORCEINLINE bool DoesObjectOriginateFromHere(const T& aObject) const
	{
		return (&aObject >= myObjectsData) && (&aObject < myObjectsData + myNumObjects);
	}

	void ReturnByIndex(const u16 aIndex) override
	{
		Return(myObjectsData[aIndex]);
	}

	FORCEINLINE ChunkedDataType* GetChunkedData() const
	{
		return static_cast<ChunkedDataType*>(Base::GetChunkedData());
	}

protected:
	FORCEINLINE u16 IndexOf(const T& aObject) const
	{
		CHECK(DoesObjectOriginateFromHere(aObject));
		return static_cast<u16>(&aObject - myObjectsData);
	}

	T* myObjectsData;
};
