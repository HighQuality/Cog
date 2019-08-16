#include "pch.h"
#include "CogClientGame.h"

#include <Cog/Widget.h>
#include <Threading/Fibers/Await.h>

#include "Program.h"

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
#include "SpriteComponent.h"

CogClientGame::CogClientGame()
{
	myNextFramesGpuCommands = new EventList<GpuCommand>();
	myCurrentlyExecutingGpuCommands = new Array<GpuCommand>();

	myWindow = new Window();
	myRenderer = nullptr;
}

CogClientGame::~CogClientGame()
{
	delete myWindow;
	myWindow = nullptr;

	delete myRenderer;
	myRenderer = nullptr;

	delete myNextFramesGpuCommands;
	myNextFramesGpuCommands = nullptr;

	delete myCurrentlyExecutingGpuCommands;
	myCurrentlyExecutingGpuCommands = nullptr;
}

bool CogClientGame::ShouldKeepRunning() const
{
	return myWindow && myWindow->IsOpen();
}

void CogClientGame::Run()
{
	myWindow->Open();

	RenderingMode renderingMode;
	renderingMode.width = myWindow->GetWidth();
	renderingMode.height = myWindow->GetHeight();
	renderingMode.fullscreen = false;
	renderingMode.vsync = true;

	myRenderer = new RenderEngine(myWindow->GetHandle(), renderingMode);

	myCamera = &CreateCamera();

	myWindow->SetVisible(true);
	myWindow->RequestFocus();

	VertexBuffer<StandardVertex> vb(*myRenderer, {
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

	VertexShader vs(*myRenderer, L"../assets/shaders/default.fx", layout);
	PixelShader ps(*myRenderer, L"../assets/shaders/default.fx");

	vs.Bind();
	ps.Bind();

	Texture texture(*myRenderer, L"../assets/textures/logo.png");

	texture.BindToPS(0);

	Base::Run();
}

void CogClientGame::SynchronizedTick(const Time& aDeltaTime)
{
	for (i32 i = 0; i < myWidgets.GetLength(); ++i)
	{
		if (!myWidgets[i])
		{
			myWidgets.RemoveAtSwap(i);
			--i;
		}
	}

	ProcessInput();

	// Gather the previous frame's GPU commands into a list for us to execute this frame
	myCurrentlyExecutingGpuCommands->Empty();
	myNextFramesGpuCommands->GatherInto(*myCurrentlyExecutingGpuCommands);

	Base::SynchronizedTick(aDeltaTime);
}

void CogClientGame::ProcessInput()
{
	myWindow->ProcessMessages();

	WindowEvent event;

	while (myWindow->PollMessage(event))
	{
		switch (event.type)
		{
		case WindowEventType::Close:
			myWindow->Close();
			break;

		case WindowEventType::Resize:
			myRenderer->Resize(event.data.resize.newWidth, event.data.resize.newHeight);
			break;

		default:
			break;
		}
	}
}

void CogClientGame::GpuExec()
{
	Array<GpuCommand>& gpuCommands = *myCurrentlyExecutingGpuCommands;

	myRenderer->ClearBackbuffer();

	for (GpuCommand& command : gpuCommands)
	{
		switch (command.type)
		{
		case GpuCommandType::DrawSprite:
			command.drawSpriteData.sprite->GpuExec();
		}
	}

	myRenderer->Draw();
	
	myRenderer->Present();
}

void CogClientGame::NewWidgetCreated(Widget& aWidget)
{
	myWidgets.Add(aWidget);
}

void CogClientGame::UpdateFrameData(FrameData& aData, const Time& aDeltaTime)
{
	aData.gpuCommands = myNextFramesGpuCommands;
}

Entity& CogClientGame::CreateCamera()
{
	EntityInitializer camera = CreateEntity();
	RenderTarget& renderTarget = camera.AddComponent<RenderTarget>();
	Entity& cameraObject = camera.Initialize();

	myRenderer->OnBackbufferRecreated.Subscribe(renderTarget, &RenderTarget::SetRenderTexture);
	renderTarget.SetRenderTexture(myRenderer->GetBackbuffer());

	return cameraObject;
}

void CogClientGame::DispatchTick()
{
	Base::DispatchTick();
	
	gProgram->QueueWork<FrameData>([](FrameData* aTickData)
	{
		NO_AWAITS;

		CogClientGame& game = GetGame<CogClientGame>();
		for (Ptr<Widget>& widget : game.myWidgets)
		{
			if (widget)
				widget->Tick(*aTickData);
		}
	}, myFrameData);
	
	Program::Get().QueueHighPrioWork<CogClientGame>([](CogClientGame* This) { This->GpuExec(); }, this);
}
