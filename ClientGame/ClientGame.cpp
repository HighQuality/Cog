#include "pch.h"
#include "ClientGame.h"
#include <Scene.h>
#include <ObjectInitializer.h>

ClientGame::ClientGame()
{
	myScene = new Scene();

	AddScene(*myScene);
}

ClientGame::~ClientGame()
{
	delete myScene;
	myScene = nullptr;
}
