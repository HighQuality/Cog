#pragma once

static constexpr i32 NumChunkObjects = static_cast<i32>(MaxOf<ChunkIndex>) + 1;

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
	ChunkedData(TArgs& ...aArgs)
	{
		const Array<i32> sizePerVariable { sizeof(*aArgs)... };

		i32 sizePerObject = 0;
		for (i32 size : sizePerVariable)
			sizePerObject += size;

		i32 sizeBytes = NumChunkObjects * sizePerObject;
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
	
	virtual void DefaultInitializeIndex(const ChunkIndex aIndex)
	{ }

	virtual void DestructIndex(const ChunkIndex aIndex)
	{ }

protected:
	FORCEINLINE static i32 GetNumObjects() { return NumChunkObjects; }

private:
	template <typename T>
	void AllocateChunkData(T*& aData)
	{
		const i32 sizeBytes = sizeof(T) * NumChunkObjects;

		CHECK(&myData[myDataPointer + sizeBytes] <= myDataEnd);

		T* data = reinterpret_cast<T*>(&myData[myDataPointer]);
		myDataPointer += sizeBytes;

		// Align next allocation to 32 bytes
		const i32 mod = myDataPointer % 32;
		if (mod > 0)
			myDataPointer += 32 - mod;

		aData = data;
	}

	u8* myData = nullptr;
	u8* myDataEnd = nullptr;
	i32 myDataPointer = 0;
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

#define DECLARE_TYPE_DATA(TTypeData) \
	public: \
	using TypeData = TTypeData;

class BaseFactoryChunk
{
public:
	BaseFactoryChunk()
		: mySlotsStatus{},
		myGeneration{},
		myIsPendingDestroy{}
	{
		for (i32 i = 0; i < NumChunkObjects; ++i)
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
		return myAllocatedObjects == NumChunkObjects;
	}

	FORCEINLINE bool IsAllObjectsActivated() const
	{
		return myDeactivatedObjects == 0;
	}

	bool IsActivated(const ChunkIndex aIndex) const
	{
		CHECK_PEDANTIC(aIndex < NumChunkObjects);
		return mySlotsStatus[aIndex] == FactoryChunkSlotStatus::Occupied;
	}

	void SetActivated(const ChunkIndex aIndex, const bool aIsActivated)
	{
		CHECK_PEDANTIC(aIndex < NumChunkObjects);
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

	FORCEINLINE u8 FindGeneration(const ChunkIndex aIndex) const
	{
		CHECK_PEDANTIC(aIndex < NumChunkObjects);
		return myGeneration[aIndex];
	}

	FORCEINLINE bool IsPendingDestroy(const ChunkIndex aIndex) const
	{
		CHECK_PEDANTIC(aIndex < NumChunkObjects);
		return myIsPendingDestroy[aIndex];
	}

	FORCEINLINE void MarkPendingDestroy(const ChunkIndex aIndex)
	{
		CHECK_PEDANTIC(aIndex < NumChunkObjects);
		myIsPendingDestroy[aIndex] = true;
	}

	FORCEINLINE ChunkedData* GetChunkedData() const { return myChunkedData; }

	virtual void InitializeIndex(const ChunkIndex aIndex)
	{
		CHECK_PEDANTIC(aIndex < NumChunkObjects);

		if (myChunkedData)
			myChunkedData->DefaultInitializeIndex(aIndex);

		++myGeneration[aIndex];
	}

	virtual void DestructIndex(const ChunkIndex aIndex)
	{
		CHECK_PEDANTIC(aIndex < NumChunkObjects);

		if (myChunkedData)
			myChunkedData->DestructIndex(aIndex);

		++myGeneration[aIndex];
	}

	virtual void ReturnByIndex(ChunkIndex aIndex) = 0;

protected:
	FactoryChunkSlotStatus mySlotsStatus[NumChunkObjects];
	u8 myGeneration[NumChunkObjects];
	bool myIsPendingDestroy[NumChunkObjects];

	ChunkedData* myChunkedData = nullptr;

	std::atomic<u16> myDeactivatedObjects;

	i32 myAllocatedObjects = 0;
	i32 myMaxOccupiedIndex = -1;
};


template <typename T>
auto GetChunkedDataTypeHelper(i32) -> typename T::ChunkData*;

template <typename T>
ChunkedData* GetChunkedDataTypeHelper(...);

template <typename T>
class FactoryChunk : public BaseFactoryChunk
{
public:
	using Base = BaseFactoryChunk;
	using ChunkedDataType = RemovePointer<decltype(GetChunkedDataTypeHelper<T>(0))>;

	FactoryChunk()
	{
		myChunkedData = &myChunkedDataStorage;
	}

	virtual ~FactoryChunk()
	{
		if (!IsEmpty())
			FATAL(L"References to this factory chunk remains!");
	}

	template <typename TCallback>
	void ForEach(const TCallback& callback) const
	{
		if (IsEmpty())
			return;

		if (!IsFull() || !IsAllObjectsActivated())
		{
			for (i32 i = 0; i <= myMaxOccupiedIndex; ++i)
			{
				if (mySlotsStatus[i] != FactoryChunkSlotStatus::Free)
					callback((&myFirstObject)[i]);
			}
		}
		else
		{
			for (i32 i = 0; i < NumChunkObjects; ++i)
				callback((&myFirstObject)[i]);
		}
	}

	virtual T& Allocate()
	{
		if (IsFull())
			FATAL(L"This factory chunk has no free room");

		++myAllocatedObjects;

		for (i32 i = 0; i < NumChunkObjects; ++i)
		{
			if (mySlotsStatus[i] == FactoryChunkSlotStatus::Free)
			{
				mySlotsStatus[i] = FactoryChunkSlotStatus::Occupied;

				if (i > myMaxOccupiedIndex)
					myMaxOccupiedIndex = i;

				Object& object = (&myFirstObject)[i];

				const ChunkIndex chunkIndex = CastBoundsChecked<ChunkIndex>(i);

				InitializeIndex(chunkIndex);

				new(static_cast<void*>(&object)) T();

				object.myChunk = this;
				object.myChunkIndex = chunkIndex;

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
		const ChunkIndex index = IndexOf(aObject);

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

		while (myMaxOccupiedIndex >= 0 && mySlotsStatus[myMaxOccupiedIndex] == FactoryChunkSlotStatus::Free)
			--myMaxOccupiedIndex;
	}

	FORCEINLINE bool DoesObjectOriginateFromHere(const T& aObject) const
	{
		return (&aObject >= &myFirstObject) && (&aObject < &myFirstObject + NumChunkObjects);
	}

	void ReturnByIndex(const ChunkIndex aIndex) override
	{
		Return((&myFirstObject)[aIndex]);
	}

	FORCEINLINE ChunkedDataType* GetChunkedData() const
	{
		return static_cast<ChunkedDataType*>(Base::GetChunkedData());
	}

protected:
	FORCEINLINE ChunkIndex IndexOf(const T& aObject) const
	{
		CHECK_PEDANTIC(DoesObjectOriginateFromHere(aObject));
		return static_cast<ChunkIndex>(&aObject - &myFirstObject);
	}

	union
	{
		mutable std::aligned_storage_t<sizeof(T[NumChunkObjects]), alignof(T[NumChunkObjects])> myObjectsStorage;
		
		mutable T myFirstObject;
	};

private:
	ChunkedDataType myChunkedDataStorage;
};
