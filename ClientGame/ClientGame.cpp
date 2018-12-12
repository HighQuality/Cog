#include "pch.h"
#include "ClientGame.h"
#include <GameWorld.h>
#include <ObjectInitializer.h>

ClientGame::ClientGame()
{
	myWorld = new GameWorld();

	AddWorld(*myWorld);
}

ClientGame::~ClientGame()
{
	delete myWorld;
	myWorld = nullptr;
}
