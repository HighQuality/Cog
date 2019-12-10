#include "CogPch.h"
#include "CogTypeChunk.h"

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
		if (auto* dispatcher = myMessageBroadcasters[aIndex])
			dispatcher(aMessage, this);
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
