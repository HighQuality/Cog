#pragma once
#include <Game/Game.h>
#include "Window.h"
#include "RenderEngine.h"
#include "Camera.h"
#include "ClientGame.generated.h"

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
	void SynchronizedTick(f32 aDeltaSeconds) override;

private:
	void ProcessInput();

	COGPROPERTY(UniquePtr<Window> Window);
	COGPROPERTY(UniquePtr<RenderEngine> Renderer, PublicRead);

	COGPROPERTY(Ptr<Camera> Camera);
};

