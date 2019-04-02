#pragma once
#include <EventList.h>

class Object;

class MessageSystem
{
	struct Message;

public:
	MessageSystem();
	~MessageSystem();

	template <typename T>
	void SendMessage(const Object& aTarget, T aMessageData)
	{
		Message msg;
		msg.target = &aTarget;
		msg.message = LinearAllocate(aMessageData);
		msg.destructMessage = [](void* aData) { static_cast<T*>(aData)->~T(); };
		msg.messageTypeId = TypeID<Message>::Resolve<T>();

		SubmitMessage(Move(msg));
	}

	bool PostMessages();

private:
	struct Message
	{
		const Object* target;
		void* message;
		void(*destructMessage)(void*);
		TypeID<Message> messageTypeId;
	};

	void SubmitMessage(Message aMessage);

	EventList<Message> myMessages;
	Map<const Object*, Array<Message>> myCurrentMessages;
};

