#pragma once
#include <Game.h>

class CogServerGame : public Game
{
public:
	using Base = Game;

	CogServerGame();
	~CogServerGame();
	
	bool ShouldKeepRunning() const override;

protected:
	void Tick(const Time& aDeltaTime) override;
	void NewWidgetCreated(Widget& aWidget) override;
};

