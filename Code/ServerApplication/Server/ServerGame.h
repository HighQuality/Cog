#pragma once
#include <Game/Game.h>
#include "ServerGame.generated.h"

COGTYPE(Specialization)
class ServerGame final : public Game
{
	GENERATED_BODY;

public:
	bool ShouldKeepRunning() const override;

	ServerGame();
	~ServerGame();
};
