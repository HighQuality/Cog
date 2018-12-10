#include "pch.h"
#include "ClientCogGame.h"
#include "Window.h"
#include "Stopwatch.h"
#include "RenderEngine.h"
#include "Texture.h"
#include "PixelShader.h"
#include "VertexShader.h"
#include "StandardVertex.h"
#include "VertexBuffer.h"
#include "InputLayout.h"

ClientCogGame::ClientCogGame()
{
	myWindow = new Window();
	myRenderer = nullptr;
}

ClientCogGame::~ClientCogGame()
{
	delete myWindow;
	myWindow = nullptr;

	delete myRenderer;
	myRenderer = nullptr;
}

bool ClientCogGame::ShouldKeepRunning() const
{
	return myWindow && myWindow->IsOpen();
}

void ClientCogGame::Run()
{
	myWindow->Open();

	RenderingMode renderingMode;
	renderingMode.width = myWindow->GetWidth();
	renderingMode.height = myWindow->GetHeight();
	renderingMode.fullscreen = false;
	renderingMode.vsync = true;

	myRenderer = new RenderEngine(myWindow->GetHandle(), renderingMode);

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

void ClientCogGame::Tick(const Time& aDeltaTime)
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

	myRenderer->ClearBackbuffer();

	myRenderer->Draw();

	myRenderer->Present();
}
