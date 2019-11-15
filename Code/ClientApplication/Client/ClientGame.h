#pragma once
#include <Game/Game.h>
#include "ClientGame.generated.h"

class Window;
class RenderEngine;
class BaseObjectFactory;

COGTYPE(Specialization)
class ClientGame final : public Game
{
	GENERATED_BODY;

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

