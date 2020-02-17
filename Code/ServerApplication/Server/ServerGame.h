#pragma once
#include <Game/Game.h>
#include "ServerGame.generated.h"

COGTYPE(Specialization)
class ServerGame final : public Game
{
	GENERATED_BODY;

protected:
	bool Starting() override;
	void ShuttingDown() override;

public:
	bool ShouldKeepRunning() const override;
};
