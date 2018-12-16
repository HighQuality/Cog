#include "pch.h"
#include "CogClientGame.h"
#include "Window.h"
#include "RenderEngine.h"
#include "Texture.h"
#include "PixelShader.h"
#include "VertexShader.h"
#include "StandardVertex.h"
#include "VertexBuffer.h"
#include "InputLayout.h"
#include <BaseComponentFactory.h>
#include <ThreadPool.h>
#include "RenderTarget.h"

CogClientGame::CogClientGame()
{
	myWindow = new Window();
	myRenderer = nullptr;
}

CogClientGame::~CogClientGame()
{
	delete myWindow;
	myWindow = nullptr;

	delete myRenderer;
	myRenderer = nullptr;
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

void CogClientGame::Tick(const Time& aDeltaTime)
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

	Base::Tick(aDeltaTime);

	myRenderer->ClearBackbuffer();

	myRenderer->Draw();

	myRenderer->Present();
}

void CogClientGame::DispatchWork(const Time& aDeltaTime)
{
	myThreadPool.Barrier();

	DispatchDraw(myCamera->GetComponent<RenderTarget>());
}

Object& CogClientGame::CreateCamera()
{
	ObjectInitializer camera = CreateObject();
	RenderTarget& renderTarget = camera.AddComponent<RenderTarget>();
	Object& cameraObject = camera.Initialize();

	myRenderer->OnBackbufferRecreated.Subscribe(renderTarget, &RenderTarget::SetRenderTexture);
	renderTarget.SetRenderTexture(myRenderer->GetBackbuffer());

	return cameraObject;
}

void CogClientGame::DispatchTick(const Time& aDeltaTime)
{
	Base::DispatchTick(aDeltaTime);

	for (BaseWidgetFactory* factory : myWidgetFactories)
	{
		if (!factory)
			continue;

		factory->IterateChunks([aDeltaTime](BaseWidgetFactoryChunk& aChunk)
		{
			aChunk.DispatchTick(aDeltaTime);
		});
	}
}

void CogClientGame::DispatchDraw(RenderTarget& aRenderTarget)
{
	for (BaseComponentFactory* factory : myComponentFactories)
	{
		if (!factory)
			continue;

		factory->IterateChunks([&aRenderTarget](BaseComponentFactoryChunk& aChunk)
		{
			aChunk.DispatchDraw3D(aRenderTarget);
		});
	}

	for (BaseComponentFactory* factory : myComponentFactories)
	{
		if (!factory)
			continue;

		factory->IterateChunks([&aRenderTarget](BaseComponentFactoryChunk& aChunk)
		{
			aChunk.DispatchDraw2D(aRenderTarget);
		});
	}

	for (BaseWidgetFactory* factory : myWidgetFactories)
	{
		if (!factory)
			continue;

		factory->IterateChunks([&aRenderTarget](BaseWidgetFactoryChunk& aChunk)
		{
			aChunk.DispatchDraw(aRenderTarget);
		});
	}
}

BaseWidgetFactory& CogClientGame::FindOrCreateWidgetFactory(const TypeID<Widget>& aWidgetType, const FunctionView<BaseWidgetFactory*()>& aFactoryCreator)
{
	CHECK(IsInGameThread());
	const u16 index = aWidgetType.GetUnderlyingInteger();
	myWidgetFactories.Resize(TypeID<Widget>::MaxUnderlyingInteger());
	auto& factory = myWidgetFactories[index];
	if (!factory)
		factory = aFactoryCreator();
	return *factory;
}
