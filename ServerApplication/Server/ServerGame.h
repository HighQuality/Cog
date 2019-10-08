#pragma once
#include <Game/Game.h>

class ServerGame final : public Game
{
public:
	using Base = Game;

	bool ShouldKeepRunning() const override;

	ServerGame();
	~ServerGame();
};
