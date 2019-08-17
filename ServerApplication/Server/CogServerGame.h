#pragma once
#include <Game/Game.h>

class CogServerGame : public Game
{
public:
	using Base = Game;

	CogServerGame();
	~CogServerGame();
	
	bool ShouldKeepRunning() const override;

protected:
	void SynchronizedTick(const Time& aDeltaTime) override;
	void NewWidgetCreated(Widget& aWidget) override;
};

