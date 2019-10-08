#include "pch.h"
#include "ClientGame.h"

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
#include "ClientTypeList.h"
#include "GpuCommand.h"

ClientGame::ClientGame()
{
	RegisterTypes<ClientTypeList>();

	myNextFramesGpuCommands = MakeUnique<EventList<GpuCommand>>();
	myCurrentlyExecutingGpuCommands = MakeUnique<Array<GpuCommand>>();

	myWindow = MakeUnique<Window>();
}

ClientGame::~ClientGame()
{
}

bool ClientGame::ShouldKeepRunning() const
{
	return myWindow && myWindow->IsOpen();
}

void ClientGame::Run()
{
	myWindow->Open();

	RenderingMode renderingMode;
	renderingMode.width = myWindow->GetWidth();
	renderingMode.height = myWindow->GetHeight();
	renderingMode.fullscreen = false;
	renderingMode.vsync = true;

	myRenderer = MakeUnique<RenderEngine>(myWindow->GetHandle(), renderingMode);

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

void ClientGame::SynchronizedTick(const Time& aDeltaTime)
{
	ProcessInput();

	// Gather the previous frame's GPU commands into a list for us to execute this frame
	myCurrentlyExecutingGpuCommands->Empty();
	myNextFramesGpuCommands->GatherInto(*myCurrentlyExecutingGpuCommands);

	Base::SynchronizedTick(aDeltaTime);
}

void ClientGame::ProcessInput()
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

void ClientGame::GpuExec()
{
	// Array<GpuCommand>& gpuCommands = *myCurrentlyExecutingGpuCommands;

	myRenderer->ClearBackbuffer();
	
	myRenderer->Draw();
	
	myRenderer->Present();
}

void ClientGame::UpdateFrameData(FrameData& aData, const Time& aDeltaTime)
{
}

Object& ClientGame::CreateCamera()
{
	Object& camera = CreateObject<Object>();
	// RenderTarget& renderTarget = camera.CreateChild<RenderTarget>();
	// 
	// myRenderer->OnBackbufferRecreated.Subscribe(renderTarget, &RenderTarget::SetRenderTexture);
	// renderTarget.SetRenderTexture(myRenderer->GetBackbuffer());

	return camera;
}

void ClientGame::DispatchTick()
{
	Base::DispatchTick();
	
	Program::Get().QueueHighPrioWork<ClientGame>([](ClientGame* This) { This->GpuExec(); }, this);
}
