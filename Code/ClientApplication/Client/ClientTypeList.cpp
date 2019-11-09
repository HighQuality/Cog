#include "ClientApplicationPch.h"
#include "ClientTypeList.h"
#include "ClientGame.h"

void ClientTypeList::RegisterTypes()
{
	Base::RegisterTypes();

	REGISTER_TYPE_SPECIALIZATION(CogGame, ClientGame);
}
