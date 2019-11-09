#pragma once
#include <Game/Game.h>

class Window;
class RenderEngine;
class BaseObjectFactory;

class ClientGame : public InheritType<Game>
{
public:
	ClientGame();
	~ClientGame();

	bool ShouldKeepRunning() const override;

	void Run() override;

protected:
	void SynchronizedTick(const Time& aDeltaTime) override;

	void DispatchTick() override;
	
	void UpdateFrameData(FrameData& aData, const Time& aDeltaTime) override;

private:
	void ProcessInput();
	void GpuExec();

	Object& CreateCamera();
	
	UniquePtr<Window> myWindow;
	UniquePtr<RenderEngine> myRenderer;
	UniquePtr<EventList<struct GpuCommand>> myNextFramesGpuCommands;
	UniquePtr<Array<struct GpuCommand>> myCurrentlyExecutingGpuCommands;

	Ptr<Object> myCamera;
};

