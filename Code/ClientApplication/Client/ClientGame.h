#pragma once
#include <Game/Game.h>
#include "Window.h"
#include "RenderEngine.h"
#include "Camera.h"
#include "GpuCommand.h"
#include "ClientGame.generated.h"

COGTYPE(Specialization)
class ClientTestType final : public TestType { GENERATED_BODY; };

COGTYPE(Specialization)
class ClientGame final : public Game
{
	GENERATED_BODY;

protected:
	bool Starting() override;
	void ShuttingDown() override;

public:
	bool ShouldKeepRunning() const override;

protected:
	void SynchronizedTick(const Time& aDeltaTime) override;

	void UpdateFrameData(FrameData& aData, const Time& aDeltaTime) override;

private:
	void ProcessInput();

	COGPROPERTY(UniquePtr<Window> Window);
	COGPROPERTY(UniquePtr<RenderEngine> Renderer, PublicRead);
	COGPROPERTY(UniquePtr<EventList<GpuCommand>> NextFramesGpuCommands);
	COGPROPERTY(UniquePtr<Array<GpuCommand>> CurrentlyExecutingGpuCommands);

	COGPROPERTY(Ptr<Camera> Camera);
};

