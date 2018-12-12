#pragma once
#include <CogClientGame.h>

class GameWorld;

class ClientGame final : public CogClientGame
{
public:
	using Base = CogClientGame;

	ClientGame();
	~ClientGame();

private:
	GameWorld* myWorld = nullptr;
};
