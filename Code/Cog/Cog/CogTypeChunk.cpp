#include "CogPch.h"
#include "CogTypeChunk.h"
#include "Pointer.h"

CogTypeChunk::CogTypeChunk()
{
	// All slots are free by default
	myFreeSlots[0] = MaxOf<u64>;
	myFreeSlots[1] = MaxOf<u64>;
	myFreeSlots[2] = MaxOf<u64>;
	myFreeSlots[3] = MaxOf<u64>;

	memset(myIsPendingDestroy, 0, sizeof myIsPendingDestroy);
}

CogTypeChunk::~CogTypeChunk()
{
}

void CogTypeChunk::Initialize()
{
	myDefaultObject = CreateDefaultObject();

	const Array<EventDispatcherInfo> interestedInEvents = GetInterestingEvents();

	i32 maxId = -1;
	for (i32 i = 0; i < interestedInEvents.GetLength(); ++i)
	{
		if (interestedInEvents[i].typeId > maxId)
			maxId = interestedInEvents[i].typeId;
	}

	if (maxId >= 0)
	{
		myMessageBroadcasters.Resize(maxId + 1);

		for (i32 i = 0; i < interestedInEvents.GetLength(); ++i)
		{
			const EventDispatcherInfo dispatcherInfo = interestedInEvents[i];
			myMessageBroadcasters[dispatcherInfo.typeId] = dispatcherInfo.messageBroadcaster;
		}

		myMessageSenders.Resize(maxId + 1);

		for (i32 i = 0; i < interestedInEvents.GetLength(); ++i)
		{
			const EventDispatcherInfo dispatcherInfo = interestedInEvents[i];
			myMessageSenders[dispatcherInfo.typeId] = dispatcherInfo.messageSender;
		}
	}
}

void CogTypeChunk::MarkPendingDestroy(const u8 aIndex)
{
	myIsPendingDestroy[aIndex] = true;
}

void CogTypeChunk::BroadcastMessageById(const Message& aMessage, const TypeID<Message>::CounterType aIndex)
{
	if (myMessageBroadcasters.IsValidIndex(aIndex))
	{
		u8 indices[256];
		const u16 length = GatherOccupiedSlots(indices);

		if (auto* dispatcher = myMessageBroadcasters[aIndex])
			dispatcher(aMessage, this, ArrayView<u8>(indices, length));
	}
}

void CogTypeChunk::SendMessageById(const Message& aMessage, const TypeID<Message>::CounterType aIndex, const u8 aReceiver)
{
	if (myMessageSenders.IsValidIndex(aIndex))
	{
		if (auto* sender = myMessageSenders[aIndex])
			sender(aMessage, this, aReceiver);
	}
}

Ptr<Object> CogTypeChunk::Allocate()
{
	u8 allocatedIndex;

	if (OccupyFirstFreeSlot(allocatedIndex))
	{
		// This looks like it creates a pointer to the default object but in reality it uses it to construct an internal "pointer object"
		Ptr<Object> obj(myDefaultObject);
		obj->myChunk = this;
		obj->myChunkIndex = allocatedIndex;
		obj->myGeneration = ++myGeneration[allocatedIndex];
		return obj;
	}

	return nullptr;
}

UniquePtr<Object> CogTypeChunk::CreateDefaultObject() const
{
	FATAL_PURE_VIRTUAL();
}

bool CogTypeChunk::OccupyFirstFreeSlot(u8& aFreeIndex)
{
	// TODO: Vectorize

	unsigned long index;

	for (;;)
	{
		const u64 first = myFreeSlots[0];

		if (_BitScanForward64(&index, first))
		{
			if (_interlockedbittestandreset64(PLONG64(&myFreeSlots[0]), index) == 0)
				continue;

			aFreeIndex = static_cast<u8>((64 * 0) + index);
			return true;
		}

		const u64 second = myFreeSlots[1];

		if (_BitScanForward64(&index, second))
		{
			if (_interlockedbittestandreset64(PLONG64(&myFreeSlots[1]), index) == 0)
				continue;

			aFreeIndex = static_cast<u8>((64 * 1) + index);
			return true;
		}

		const u64 third = myFreeSlots[2];

		if (_BitScanForward64(&index, third))
		{
			if (_interlockedbittestandreset64(PLONG64(&myFreeSlots[2]), index) == 0)
				continue;

			aFreeIndex = static_cast<u8>((64 * 2) + index);
			return true;
		}

		const u64 fourth = myFreeSlots[3];

		if (_BitScanForward64(&index, fourth))
		{
			if (_interlockedbittestandreset64(PLONG64(&myFreeSlots[3]), index) == 0)
				continue;

			aFreeIndex = static_cast<u8>((64 * 3) + index);
			return true;
		}

		return false;
	}
}

// void CogTypeChunk::CallThing()
// {
// 	Resource r;
// 	r.myChunk = this;
// 	
// 	u8 indices[256];
// 	u16 count = GatherOccupiedSlots(indices);
// 
// 	for (u16 i = 0; i < count; ++i)
// 	{
// 		r.myChunkIndex = indices[i];
// 
// 		r.Thing();
// 	}
// }

// TODO: Benchmark GatherOccupiedIndicesFromByte and GatherOccupiedIndicesFromU64 with and without FORCEINLINE

FORCEINLINE static void GatherOccupiedIndicesFromByte(u8* aOccupiedSlots, u16& aNextFreeIndex, const u8 aByte, const u8 aOffset)
{
	if (aByte == 0)
		return;

	if (aByte == 255)
	{
		const u8 firstFreeIndex = static_cast<u8>(aNextFreeIndex);
		aNextFreeIndex += 8;

		aOccupiedSlots[firstFreeIndex + 0] = aOffset + 0;
		aOccupiedSlots[firstFreeIndex + 1] = aOffset + 1;
		aOccupiedSlots[firstFreeIndex + 2] = aOffset + 2;
		aOccupiedSlots[firstFreeIndex + 3] = aOffset + 3;
		aOccupiedSlots[firstFreeIndex + 4] = aOffset + 4;
		aOccupiedSlots[firstFreeIndex + 5] = aOffset + 5;
		aOccupiedSlots[firstFreeIndex + 6] = aOffset + 6;
		aOccupiedSlots[firstFreeIndex + 7] = aOffset + 7;
		return;
	}

	// TODO: Table lookup?
	if (aByte & 0b00001111)
	{
		if (aByte & 0b00000011)
		{
			if (aByte & 0b00000001)
				aOccupiedSlots[aNextFreeIndex++] = aOffset + 0;

			if (aByte & 0b00000010)
				aOccupiedSlots[aNextFreeIndex++] = aOffset + 1;
		}

		if (aByte & 0b00001100)
		{
			if (aByte & 0b00000100)
				aOccupiedSlots[aNextFreeIndex++] = aOffset + 2;

			if (aByte & 0b00001000)
				aOccupiedSlots[aNextFreeIndex++] = aOffset + 3;
		}
	}
	
	if (aByte & 0b11110000)
	{
		if (aByte & 0b00110000)
		{
			if (aByte & 0b00010000)
				aOccupiedSlots[aNextFreeIndex++] = aOffset + 4;

			if (aByte & 0b00100000)
				aOccupiedSlots[aNextFreeIndex++] = aOffset + 5;
		}
		
		if (aByte & 0b11000000)
		{
			if (aByte & 0b01000000)
				aOccupiedSlots[aNextFreeIndex++] = aOffset + 6;
			
			if (aByte & 0b10000000)
				aOccupiedSlots[aNextFreeIndex++] = aOffset + 7;
		}
	}
}

static void GatherOccupiedIndicesFromU64(u8* aOccupiedSlots, u16& aNextFreeIndex, const u64 aPart, const u8 aOffset)
{
	if (aPart == 0)
		return;

	constexpr u64 firstHalf = 0b1111111111111111111111111111111100000000000000000000000000000000;
	constexpr u64 secondHalf = 0b0000000000000000000000000000000011111111111111111111111111111111;
	constexpr u64 firstQuarter = 0b1111111111111111000000000000000000000000000000000000000000000000;
	constexpr u64 secondQuarter = 0b0000000000000000111111111111111100000000000000000000000000000000;
	constexpr u64 thirdQuarter = 0b0000000000000000000000000000000011111111111111110000000000000000;
	constexpr u64 fourthQuarter = 0b0000000000000000000000000000000000000000000000001111111111111111;

	if (aPart & secondHalf)
	{
		if (aPart & fourthQuarter)
		{
			GatherOccupiedIndicesFromByte(aOccupiedSlots, aNextFreeIndex, *(reinterpret_cast<const u8*>(&aPart) + 7), aOffset + 8 * 0);
			GatherOccupiedIndicesFromByte(aOccupiedSlots, aNextFreeIndex, *(reinterpret_cast<const u8*>(&aPart) + 6), aOffset + 8 * 1);
		}

		if (aPart & thirdQuarter)
		{
			GatherOccupiedIndicesFromByte(aOccupiedSlots, aNextFreeIndex, *(reinterpret_cast<const u8*>(&aPart) + 5), aOffset + 8 * 2);
			GatherOccupiedIndicesFromByte(aOccupiedSlots, aNextFreeIndex, *(reinterpret_cast<const u8*>(&aPart) + 4), aOffset + 8 * 3);
		}
	}
	
	if (aPart & firstHalf)
	{
		if (aPart & secondQuarter)
		{
			GatherOccupiedIndicesFromByte(aOccupiedSlots, aNextFreeIndex, *(reinterpret_cast<const u8*>(&aPart) + 3), aOffset + 8 * 4);
			GatherOccupiedIndicesFromByte(aOccupiedSlots, aNextFreeIndex, *(reinterpret_cast<const u8*>(&aPart) + 2), aOffset + 8 * 5);
		}
		
		if (aPart & firstQuarter)
		{
			GatherOccupiedIndicesFromByte(aOccupiedSlots, aNextFreeIndex, *(reinterpret_cast<const u8*>(&aPart) + 1), aOffset + 8 * 6);
			GatherOccupiedIndicesFromByte(aOccupiedSlots, aNextFreeIndex, *(reinterpret_cast<const u8*>(&aPart) + 0), aOffset + 8 * 7);
		}
	}
}

u16 CogTypeChunk::GatherOccupiedSlots(u8* aOccupiedSlots) const
{
	u16 nextFreeIndex = 0;

	// TODO: Vectorize bitwise nots (and possibly way more?)

	GatherOccupiedIndicesFromU64(aOccupiedSlots, nextFreeIndex, ~myFreeSlots[0], 0 * 64);
	GatherOccupiedIndicesFromU64(aOccupiedSlots, nextFreeIndex, ~myFreeSlots[1], 1 * 64);
	GatherOccupiedIndicesFromU64(aOccupiedSlots, nextFreeIndex, ~myFreeSlots[2], 2 * 64);
	GatherOccupiedIndicesFromU64(aOccupiedSlots, nextFreeIndex, ~myFreeSlots[3], 3 * 64);

	return nextFreeIndex;
}

void CogTypeChunk::ReturnByIndex(const u8 aIndex)
{
	const u8 part = aIndex >> 6;
	const u8 partStart = part << 6;
	const u8 indexInPart = aIndex - partStart;
	const u8 previousFreeStatus = _interlockedbittestandset64(PLONG64(&myFreeSlots[part]), indexInPart);

	CHECK_MSG(previousFreeStatus == 0, L"This object was returned multiple times or never allocated");

	++myGeneration[aIndex];
}
