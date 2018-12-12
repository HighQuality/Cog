#pragma once
#include <Game.h>

class Window;
class RenderEngine;

class CogClientGame : public Game
{
public:
	using Base = Game;

	CogClientGame();
	~CogClientGame();

	bool ShouldKeepRunning() const override;

	void Run() override;

protected:
	void Tick(const Time& aDeltaTime) override;

private:
	Window* myWindow = nullptr;
	RenderEngine* myRenderer = nullptr;
};

