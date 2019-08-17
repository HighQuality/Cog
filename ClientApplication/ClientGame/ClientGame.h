#pragma once
#include <Client/CogClientGame.h>

class ClientGame final : public CogClientGame
{
public:
	using Base = CogClientGame;

	ClientGame();
	~ClientGame();
};
