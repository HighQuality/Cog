#pragma once
#include <Containers/EventList.h>
#include "ScheduledMessage.h"
#include "MessageSystem.generated.h"

class Object;

COGTYPE()
class MessageSystem : public Object
{
	GENERATED_BODY;

	struct Message;

public:
	template <typename T>
	void SendMessage(const Object& aTarget, T aMessageData)
	{
		ScheduledMessage msg;
		msg.target = &aTarget;
		msg.message = LinearAllocate(aMessageData);
		msg.destructMessage = [](void* aData) { static_cast<T*>(aData)->~T(); };
		msg.messageTypeId = &TypeID<ScheduledMessage>::Resolve<T>();

		SubmitMessage(Move(msg));
	}

	bool PostMessages();

private:
	void SubmitMessage(ScheduledMessage aMessage);

	COGPROPERTY(EventList<ScheduledMessage> Messages, DirectAccess);
	COGPROPERTY(Map<const Object*, Array<ScheduledMessage>> CurrentMessages, DirectAccess);
};
