#include "pch.h"
#include "ServerGame.h"
#include "ServerTypeList.h"

ServerGame::ServerGame()
{
	RegisterTypes<ServerTypeList>();
}

bool ServerGame::ShouldKeepRunning() const
{
	return true;
}

ServerGame::~ServerGame()
{
}
