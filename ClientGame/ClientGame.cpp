#include "pch.h"
#include "ClientGame.h"
#include <GameWorld.h>
#include <ObjectInitializer.h>

ClientGame::ClientGame()
{
	myWorld = new GameWorld();

	auto object = myWorld->CreateObject();
}

ClientGame::~ClientGame()
{
	delete myWorld;
	myWorld = nullptr;
}
