#pragma once
#include "CogArray.h"
#include "BitsetForEachTables.h"

class Bitset
{
public:
	Bitset()
	{
		memset(myDefaultData, 0, sizeof myDefaultData);
		myData = myDefaultData;
		myMarker = 0;
		myCapacity = StaticAllocationSize * 64;
	}

	void Resize()
	{

	}
	
	void Push(const bool aValue)
	{
		if (myMarker == myCapacity)
		{
			const bool wasUsingDefaultData = myData == myDefaultData;
			myCapacity *= 2;
			myDynamicData.Resize(myCapacity / 64);
			myData = myDynamicData.GetData();
			
			if (wasUsingDefaultData)
			{
				memcpy(myData, myDefaultData, sizeof myDefaultData);
			}
		}

		const u32 assignedIndex = myMarker;
		++myMarker;
		
		if (aValue)
			Set(assignedIndex);
		else
			Clear(assignedIndex);
	}

	void Pop()
	{
		CHECK(myMarker > 0);
		Clear(myMarker);
		--myMarker;
	}

	void Set(u32 aIndex)
	{
		CHECK(aIndex < myMarker);
		const u32 intIndex = aIndex / 64;
		const u32 bitIndex = aIndex % 64;
		myData[intIndex] |= 1ULL << bitIndex;
	}

	void Clear(u32 aIndex)
	{
		CHECK(aIndex < myMarker);
		const u32 intIndex = aIndex / 64;
		const u32 bitIndex = aIndex % 64;
		myData[intIndex] &= ~(1ULL << bitIndex);
	}
	
	bool IsSet(u32 aIndex) const
	{
		CHECK(aIndex < myMarker);
		const u32 intIndex = aIndex / 64;
		const u32 bitIndex = aIndex % 64;
		return myData[intIndex] & (1ULL << bitIndex);
	}

	template <typename TCallback>
	void ForEachSetBit(TCallback aCallback);

private:
	static constexpr i32 StaticAllocationSize = 16;
	u64* myData;
	u32 myMarker;
	u32 myCapacity;

	Array<u64> myDynamicData;
	u64 myDefaultData[StaticAllocationSize];
};


template <typename TCallback>
void Bitset::ForEachSetBit(TCallback aCallback)
{
	auto doCallback = [aCallback](const u32* aIndexMem, const u8 aLength)
	{
		u8 num = 0;
		switch (aLength)
		{
		case 8: aCallback(aIndexMem[num++]); // OR: aCallback(aIndexMem[7]);
		case 7: aCallback(aIndexMem[num++]); // OR: aCallback(aIndexMem[6]);
		case 6: aCallback(aIndexMem[num++]); // OR: aCallback(aIndexMem[5]);
		case 5: aCallback(aIndexMem[num++]); // OR: aCallback(aIndexMem[4]);
		case 4: aCallback(aIndexMem[num++]); // OR: aCallback(aIndexMem[3]);
		case 3: aCallback(aIndexMem[num++]); // OR: aCallback(aIndexMem[2]);
		case 2: aCallback(aIndexMem[num++]); // OR: aCallback(aIndexMem[1]);
		case 1: aCallback(aIndexMem[num]);	 // OR: aCallback(aIndexMem[0]);
		case 0:
			break;

		default:
			__assume(0);
		}
	};

	__m256i baseVec = _mm256_set1_epi32(-1);
	__m256i incVec = _mm256_set1_epi32(64);
	__m256i add8 = _mm256_set1_epi32(8);

	const i32 capacity = myCapacity / 64;
	const u64* data = myData;

	for (int i = 0; i < capacity; ++i)
	{
		u64 w = data[i];
		
		if (w == 0)
		{
			baseVec = _mm256_add_epi32(baseVec, incVec);
		}
		else
		{
			for (i32 k = 0; k < 4; ++k)
			{
				const u8 byteA = (u8)w;
				const u8 byteB = (u8)(w >> 8);
				w >>= 16;
				__m256i vecA = _mm256_load_si256(reinterpret_cast<const __m256i*>(gForeachDecodeTable[byteA]));
				__m256i vecB = _mm256_load_si256(reinterpret_cast<const __m256i*>(gForeachDecodeTable[byteB]));
				u8 lengthA = gForeachLengthTable[byteA];
				u8 lengthB = gForeachLengthTable[byteB];
				vecA = _mm256_add_epi32(baseVec, vecA);
				baseVec = _mm256_add_epi32(baseVec, add8);
				vecB = _mm256_add_epi32(baseVec, vecB);
				baseVec = _mm256_add_epi32(baseVec, add8);

				doCallback(reinterpret_cast<const uint32_t*>(&vecA), lengthA);
				doCallback(reinterpret_cast<const uint32_t*>(&vecB), lengthB);
			}
		}
	}
}
