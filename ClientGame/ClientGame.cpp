#include "pch.h"
#include "ClientGame.h"
#include "ClientGameComponentList.h"
#include <TestComponent.h>

ClientGame::ClientGame()
{
	RegisterComponents<ClientGameComponentList>();

	CreateObject().AddComponent<TestComponent>();
}

ClientGame::~ClientGame()
{
}
