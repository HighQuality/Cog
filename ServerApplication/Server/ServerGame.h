#pragma once
#include <Game/Game.h>

class ServerGame final : public InheritType<Game>
{
public:
	bool ShouldKeepRunning() const override;

	ServerGame();
	~ServerGame();
};
