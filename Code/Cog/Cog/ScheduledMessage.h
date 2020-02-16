#pragma once

struct ScheduledMessage
{
	const Object* target;
	void* message;
	void(*destructMessage)(void*);
	const TypeID<Message>* messageTypeId;
};
