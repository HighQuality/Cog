#include "ServerApplicationPch.h"
#include "ServerGame.h"

bool ServerGame::Starting()
{
	if (!Base::Starting())
		return false;

	return true;
}

void ServerGame::ShuttingDown()
{
	Base::ShuttingDown();
}

bool ServerGame::ShouldKeepRunning() const
{
	return true;
}
