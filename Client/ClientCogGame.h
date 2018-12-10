#pragma once
#include <CogGame.h>

class Window;
class RenderEngine;

class ClientCogGame : public CogGame
{
public:
	using Base = CogGame;

	ClientCogGame();
	~ClientCogGame();

	bool ShouldKeepRunning() const override;

	void Run() override;

protected:
	void Tick(const Time& aDeltaTime) override;

private:
	Window* myWindow = nullptr;
	RenderEngine* myRenderer = nullptr;
};

