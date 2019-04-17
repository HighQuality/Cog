#pragma once
#include <Game/Game.h>

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
	void SynchronizedTick(const Time& aDeltaTime) override;

	void DispatchTick() override;
	
	void NewWidgetCreated(Widget& aWidget) override;
	void UpdateFrameData(FrameData& aData, const Time& aDeltaTime) override;

private:
	void ProcessInput();
	void GpuExec();

	Entity& CreateCamera();
	
	Window* myWindow = nullptr;
	RenderEngine* myRenderer = nullptr;
	EventList<struct GpuCommand>* myNextFramesGpuCommands;
	Array<struct GpuCommand>* myCurrentlyExecutingGpuCommands;

	Array<Ptr<Widget>> myWidgets;

	Ptr<Entity> myCamera;
};

