#include "ServerApplicationPch.h"
#include "ServerTypeList.h"
#include "ServerGame.h"

void ServerTypeList::RegisterTypes()
{
	Base::RegisterTypes();

	REGISTER_TYPE_SPECIALIZATION(CogGame, ServerGame);
}
