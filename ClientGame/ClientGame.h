#pragma once
#include <CogClientGame.h>

class GameWorld;

class ClientGame final : public CogClientGame
{
public:
	ClientGame();
	~ClientGame();

private:
	GameWorld* myWorld = nullptr;
};
