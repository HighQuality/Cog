#include "CogPch.h"
#include "MessageSystem.h"
#include "Program.h"

bool MessageSystem::PostMessages()
{
	// Program& program = *gProgram;

	Array<ScheduledMessage> messages = GetMessages().Gather();

	Map<const Object*, Array<ScheduledMessage>>& currentMessages = GetCurrentMessages();
	currentMessages.Clear();

	for (ScheduledMessage& aMessage : messages)
		currentMessages.FindOrAdd(aMessage.target).Add(Move(aMessage));

	// for (KeyValuePair<const Object*, Array<ScheduledMessage>>& pairs : myCurrentMessages)
	// {
	// 	program.QueueWork<Array<ScheduledMessage>>([](Array<ScheduledMessage>* aMessageList)
	// 	{
	// 		for (ScheduledMessage& message : *aMessageList)
	// 		{
	// 			message.target->ReceiveRawMessage(message.message, message.messageTypeId);
	// 			message.destructMessage(message.message);
	// 		}
	// 	}, &pairs.value);
	// }

	return false;
}

void MessageSystem::SubmitMessage(ScheduledMessage aMessage)
{
	// Needed to avoid locking in PostMessages and here
	CHECK(GetProgram().IsInManagedThread());
	GetMessages().Submit(Move(aMessage));
}
