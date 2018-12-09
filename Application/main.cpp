#include "pch.h"
#include <iostream>
#include "BinaryData.h"
#include "Factory.h"
#include "Window.h"
#include "Stopwatch.h"
#include "GameWorld.h"
#include "Pointer.h"

int main()
{
	gThreadID = 1;

	Window window;
	window.Open();
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
	}

	return 0;
}
