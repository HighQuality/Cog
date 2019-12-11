#pragma once

class Message;
class CogTypeChunk;

struct EventDispatcherInfo
{
	void(*messageBroadcaster)(const Message&, CogTypeChunk*) = nullptr;
	void(*messageSender)(const Message&, CogTypeChunk*, u8) = nullptr;
	TypeID<Message>::CounterType typeId;
};

class CogTypeChunk
{
public:
	CogTypeChunk();
	virtual ~CogTypeChunk();

	DELETE_COPYCONSTRUCTORS_AND_MOVES(CogTypeChunk);

	virtual void Initialize();

	FORCEINLINE bool IsPendingDestroy(const u8 aIndex) const { return myIsPendingDestroy[aIndex]; }

	virtual Array<EventDispatcherInfo> GetInterestingEvents() const { return Array<EventDispatcherInfo>(); }

	template <typename T>
	void BroadcastMessage(const T& aMessage)
	{
		BroadcastMessageById(aMessage, TypeID<Message>::Resolve<T>().GetUnderlyingInteger());
	}

	void BroadcastMessageById(const Message& aMessage, TypeID<Message>::CounterType aIndex);

	template <typename T>
	void SendMessage(const T& aMessage, const u8 aReceiver)
	{
		SendMessageById(aMessage, TypeID<Message>::Resolve<T>().GetUnderlyingInteger(), aReceiver);
	}

	void SendMessageById(const Message& aMessage, TypeID<Message>::CounterType aIndex, u8 aReceiver);

private:
	Array<void(*)(const Message&, CogTypeChunk*, ArrayView<u8>)> myMessageBroadcasters;
	Array<void(*)(const Message&, CogTypeChunk*, u8)> myMessageSenders;

	bool OccupyFirstFreeSlot(u8& aFreeIndex);
	/** Returns number of elements filled in aOccupiedSlots output */
	u16 GatherOccupiedSlots(u8* aOccupiedSlots) const;

	volatile u64 myFreeSlots[4];
	bool myIsPendingDestroy[256];
};
