#include "pch.h"
#include "Window.h"
#include "Stopwatch.h"
#include "RenderEngine.h"
#include "StandardVertex.h"
#include "VertexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"

int main()
{
	gThreadID = 1;

	Window window;
	window.Open();

	RenderingMode renderingMode;
	renderingMode.width = window.GetWidth();
	renderingMode.height = window.GetHeight();
	renderingMode.fullscreen = false;
	renderingMode.vsync = false;

	RenderEngine renderer(window.GetHandle(), renderingMode);

	VertexBuffer<StandardVertex> vb(renderer, {
		{ { 0.f, 1.f, 0.5f, 1.f }, LinearColor(1.f, 0.f, 0.f), {0.f, 0.f} },
		{ { 1.f, -1.f, 0.5f, 1.f }, LinearColor(0.f, 1.f, 0.f), {0.f, 0.f} },
		{ { -1.f, -1.f, 0.5f, 1.f }, LinearColor(0.f, 0.f, 1.f), {0.f, 0.f} }
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

	window.SetVisible(true);
	window.RequestFocus();

	Stopwatch watch;
	bool isFirstFrame = true;

	while (window.IsOpen())
	{
		Time deltaTime;

		if (isFirstFrame)
		{
			deltaTime = Time::Seconds(1.f / 60.f);
			isFirstFrame = false;
		}
		else
		{
			deltaTime = watch.GetElapsedTime();
		}

		watch.Restart();

		window.ProcessMessages();

		WindowEvent event;

		while (window.PollMessage(event))
		{
			switch (event.type)
			{
			case WindowEventType::Close:
				window.Close();
				break;

			case WindowEventType::Resize:
				renderer.Resize(event.data.resize.newWidth, event.data.resize.newHeight);
				break;
			}
		}

		renderer.ClearBackbuffer();

		renderer.Draw();

		renderer.Present();
	}

	return 0;
}
