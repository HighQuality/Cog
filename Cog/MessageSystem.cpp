#include "pch.h"
#include "MessageSystem.h"
#include <Program.h>


MessageSystem::MessageSystem()
{
}


MessageSystem::~MessageSystem()
{
}

bool MessageSystem::PostMessages()
{
	Program& program = *gProgram;

	Array<Message> messages = myMessages.Gather();

	myCurrentMessages.Clear();

	for (Message& aMessage : messages)
		myCurrentMessages.FindOrAdd(aMessage.target).Add(Move(aMessage));

	// for (KeyValuePair<const Object*, Array<Message>>& pairs : myCurrentMessages)
	// {
	// 	program.QueueWork<Array<Message>>([](Array<Message>* aMessageList)
	// 	{
	// 		for (Message& message : *aMessageList)
	// 		{
	// 			message.target->ReceiveRawMessage(message.message, message.messageTypeId);
	// 			message.destructMessage(message.message);
	// 		}
	// 	}, &pairs.value);
	// }

	return false;
}

void MessageSystem::SubmitMessage(Message aMessage)
{
	// Needed to avoid locking in PostMessages and here
	CHECK(gProgram->IsInManagedThread());
	myMessages.Submit(Move(aMessage));
}
