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
	CogTypeChunk() = default;
	virtual ~CogTypeChunk() = default;

	DELETE_COPYCONSTRUCTORS_AND_MOVES(CogTypeChunk);
	
	virtual Array<EventDispatcherInfo> GetInterestingEvents() const { return Array<EventDispatcherInfo>(); }

	virtual void Initialize();

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
	Array<void(*)(const Message&, CogTypeChunk*)> myMessageBroadcasters;
	Array<void(*)(const Message&, CogTypeChunk*, u8)> myMessageSenders;
};
