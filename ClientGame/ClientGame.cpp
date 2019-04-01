#include "pch.h"
#include "ClientGame.h"
#include "ClientGameComponentList.h"
#include <TestComponent.h>

class Transform2D;

ClientGame::ClientGame()
{
	RegisterComponents<ClientGameComponentList>();

	auto entity = CreateEntity();
	entity.AddComponent<TestComponent>();
	entity.AddComponent<Transform2D>();
}

ClientGame::~ClientGame()
{
}
