#pragma once
#include <CogClientGame.h>

class Scene;

class ClientGame final : public CogClientGame
{
public:
	using Base = CogClientGame;

	ClientGame();
	~ClientGame();

private:
	Scene* myScene = nullptr;
};
