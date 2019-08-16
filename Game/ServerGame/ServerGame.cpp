#include "pch.h"
#include "ServerGame.h"
#include "ServerGameComponentList.h"

ServerGame::ServerGame()
{
	RegisterComponents<ServerGameComponentList>();
}

ServerGame::~ServerGame()
{
}
