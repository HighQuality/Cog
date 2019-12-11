#include "CogPch.h"
#include "CogTypeChunk.h"

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

bool CogTypeChunk::OccupyFirstFreeSlot(u8& aFreeIndex)
{
	unsigned long index;

	for (;;)
	{
		const u64 first = myFreeSlots[0];

		if (_BitScanReverse64(&index, first))
		{
			if (_interlockedbittestandset64(PLONG64(&myFreeSlots[0]), index) == 1)
				continue;

			aFreeIndex = (64 * 0) + index;
			return true;
		}

		const u64 second = myFreeSlots[1];

		if (_BitScanReverse64(&index, second))
		{
			if (_interlockedbittestandset64(PLONG64(&myFreeSlots[1]), index) == 1)
				continue;

			aFreeIndex = (64 * 1) + index;
			return true;
		}

		const u64 third = myFreeSlots[2];

		if (_BitScanReverse64(&index, third))
		{
			if (_interlockedbittestandset64(PLONG64(&myFreeSlots[2]), index) == 0)
				continue;

			aFreeIndex = (64 * 2) + index;
			return true;
		}

		const u64 fourth = myFreeSlots[3];

		if (_BitScanReverse64(&index, fourth))
		{
			if (_interlockedbittestandset64(PLONG64(&myFreeSlots[3]), index) == 0)
				continue;

			aFreeIndex = (64 * 3) + index;
			return true;
		}

		return false;
	}
}

u16 CogTypeChunk::GatherOccupiedSlots(u8* aOccupiedSlots) const
{
	u16 nextFreeIndex = 0;

	auto testByte = [&nextFreeIndex, aOccupiedSlots](const u8 aByte, const u8 aOffset)
	{
		if (aByte == 0)
			return;

		if (aByte == 255)
		{
			const u8 firstFreeIndex = nextFreeIndex;
			nextFreeIndex += 8;

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

		if (aByte & 0b11110000)
		{
			if (aByte & 0b11000000)
			{
				if (aByte & 0b10000000)
					aOccupiedSlots[nextFreeIndex++] = aOffset + 0;

				if (aByte & 0b01000000)
					aOccupiedSlots[nextFreeIndex++] = aOffset + 1;
			}

			if (aByte & 0b00110000)
			{
				if (aByte & 0b00100000)
					aOccupiedSlots[nextFreeIndex++] = aOffset + 2;

				if (aByte & 0b00010000)
					aOccupiedSlots[nextFreeIndex++] = aOffset + 3;
			}
		}
		
		if (aByte & 0b00001111)
		{
			if (aByte & 0b00001100)
			{
				if (aByte & 0b00001000)
					aOccupiedSlots[nextFreeIndex++] = aOffset + 4;

				if (aByte & 0b00000100)
					aOccupiedSlots[nextFreeIndex++] = aOffset + 5;
			}

			if (aByte & 0b00000011)
			{
				if (aByte & 0b00000010)
					aOccupiedSlots[nextFreeIndex++] = aOffset + 6;

				if (aByte & 0b00000001)
					aOccupiedSlots[nextFreeIndex++] = aOffset + 7;
			}
		}
	};

	auto test64 = [testByte](const u64 aPart)
	{
		if (aPart == 0)
			return;

		constexpr u64 firstHalf =		0b1111111111111111111111111111111100000000000000000000000000000000;
		constexpr u64 secondHalf =		0b0000000000000000000000000000000011111111111111111111111111111111;
		constexpr u64 firstQuarter =	0b1111111111111111000000000000000000000000000000000000000000000000;
		constexpr u64 secondQuarter =	0b0000000000000000111111111111111100000000000000000000000000000000;
		constexpr u64 thirdQuarter =	0b0000000000000000000000000000000011111111111111110000000000000000;
		constexpr u64 fourthQuarter =	0b0000000000000000000000000000000000000000000000001111111111111111;

		if (aPart & firstHalf)
		{
			if (aPart & firstQuarter)
			{
				testByte(*(reinterpret_cast<const u8*>(&aPart) + 0), 8 * 0);
				testByte(*(reinterpret_cast<const u8*>(&aPart) + 1), 8 * 1);
			}

			if (aPart & secondQuarter)
			{
				testByte(*(reinterpret_cast<const u8*>(&aPart) + 2), 8 * 2);
				testByte(*(reinterpret_cast<const u8*>(&aPart) + 3), 8 * 3);
			}
		}

		if (aPart & secondHalf)
		{
			if (aPart & thirdQuarter)
			{
				testByte(*(reinterpret_cast<const u8*>(&aPart) + 4), 8 * 4);
				testByte(*(reinterpret_cast<const u8*>(&aPart) + 5), 8 * 5);
			}

			if (aPart & fourthQuarter)
			{
				testByte(*(reinterpret_cast<const u8*>(&aPart) + 6), 8 * 6);
				testByte(*(reinterpret_cast<const u8*>(&aPart) + 7), 8 * 7);
			}
		}
	};

	test64(~myFreeSlots[0]);
	test64(~myFreeSlots[1]);
	test64(~myFreeSlots[2]);
	test64(~myFreeSlots[3]);

	return nextFreeIndex;
}
