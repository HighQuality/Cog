#include "pch.h"
#include "ClientGame.h"
#include <Scene.h>
#include <TestComponent.h>

ClientGame::ClientGame()
{
	myScene = new Scene(*this);
	
	AddScene(*myScene);

	auto obj = myScene->CreateObject();
	obj.AddComponent<TestComponent>();
}

ClientGame::~ClientGame()
{
	delete myScene;
	myScene = nullptr;
}
