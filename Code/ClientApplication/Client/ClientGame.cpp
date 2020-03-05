#include "ClientApplicationPch.h"
#include "ClientGame.h"
#include <Cog/Threading/Fibers/Await.h>
#include <Cog/Program.h>

#include "Window.h"
#include "RenderEngine.h"
#include "Texture.h"
#include "PixelShader.h"
#include "VertexShader.h"
#include "StandardVertex.h"
#include "VertexBuffer.h"
#include "InputLayout.h"
#include "RenderTarget.h"
#include "GpuCommand.h"

bool ClientGame::ShouldKeepRunning() const
{
	return GetWindow() && GetWindow()->IsOpen();
}

bool ClientGame::Starting()
{
	if (!Base::Starting())
		return false;

	SetNextFramesGpuCommands(MakeUnique<EventList<GpuCommand>>());
	SetCurrentlyExecutingGpuCommands(MakeUnique<Array<GpuCommand>>());
	
	Window& window = *SetWindow(MakeUnique<Window>());
	window.Open();

	RenderingMode renderingMode;
	renderingMode.width = window.GetWidth();
	renderingMode.height = window.GetHeight();
	renderingMode.fullscreen = false;
	renderingMode.vsync = true;

	RenderEngine& renderer = *SetRenderer(MakeUnique<RenderEngine>(GetWindow()->GetHandle(), renderingMode));

	SetCamera(NewObject<Camera>(nullptr));

	window.SetVisible(true);
	window.RequestFocus();

	VertexBuffer<StandardVertex> vb(renderer, {
		{ { -1.f, 1.f, 0.5f, 1.f }, LinearColor::White, {0.f, 0.f} },
		{ { 1.f, 1.f, 0.5f, 1.f }, LinearColor::White, {1.f, 0.f} },
		{ { 1.f, -1.f, 0.5f, 1.f }, LinearColor::White, {1.f, 1.f} },

		{ { 1.f, -1.f, 0.5f, 1.f }, LinearColor::White, {1.f, 1.f} },
		{ { -1.f, -1.f, 0.5f, 1.f }, LinearColor::White, {0.f, 1.f} },
		{ { -1.f, 1.f, 0.5f, 1.f }, LinearColor::White, {0.f, 0.f} }
		});

	vb.Bind(0);

	InputLayout layout;
	layout.Add("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
	layout.Add("INSTANCE_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
	layout.Add("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT);

	VertexShader vs(renderer, L"../assets/shaders/default.fx", layout);
	PixelShader ps(renderer, L"../assets/shaders/default.fx");

	vs.Bind();
	ps.Bind();

	Texture texture(renderer, L"../assets/textures/logo.png");

	texture.BindToPS(0);

	return true;
}

void ClientGame::ShuttingDown()
{
	if (Camera* camera = GetCamera())
		camera->Destroy();

	Base::ShuttingDown();
}

void ClientGame::SynchronizedTick(const f32 aDeltaSeconds)
{
	ProcessInput();

	// Gather the previous frame's GPU commands into a list for us to execute this frame
	GetCurrentlyExecutingGpuCommands()->Empty();
	GetNextFramesGpuCommands()->GatherInto(*GetCurrentlyExecutingGpuCommands());

	Base::SynchronizedTick(aDeltaSeconds);

	RenderEngine* renderer = GetRenderer();

	if (ENSURE(renderer))
	{
		renderer->ClearBackbuffer();

		renderer->Draw();

		renderer->Present();
	}
}

void ClientGame::ProcessInput()
{
	Window* window = GetWindow();
	if (!ENSURE(window))
		return;

	window->ProcessMessages();

	WindowEvent event;

	while (window->PollMessage(event))
	{
		switch (event.type)
		{
		case WindowEventType::Close:
			window->Close();
			break;

		case WindowEventType::Resize:
			GetRenderer()->Resize(event.data.resize.newWidth, event.data.resize.newHeight);
			break;

		default:
			break;
		}
	}
}
