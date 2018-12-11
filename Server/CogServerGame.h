#pragma once
#include <Game.h>

class CogServerGame : public Game
{
public:
	CogServerGame();
	~CogServerGame();
	
	bool ShouldKeepRunning() const override;

protected:
	void Tick(const Time& aDeltaTime) override;
};

