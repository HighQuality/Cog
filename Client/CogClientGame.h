#pragma once
#include <Game.h>

class Window;
class RenderEngine;
class BaseObjectFactory;

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

	void DispatchTick(const Time& aDeltaTime) override;
	void DispatchDraw(RenderTarget& aRenderTarget);

	void DispatchWork(const Time& aDeltaTime) override;

	void NewWidgetCreated(Widget& aWidget) override;

private:
	Entity& CreateCamera();

	Window* myWindow = nullptr;
	RenderEngine* myRenderer = nullptr;

	Array<Ptr<Widget>> myWidgets;

	Ptr<Entity> myCamera;
};

