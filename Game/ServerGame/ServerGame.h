#pragma once
#include <Server/CogServerGame.h>

class ServerGame final : public CogServerGame
{
public:
	using Base = CogServerGame;

	ServerGame();
	~ServerGame();
};
