#pragma once
#include <Containers/EventList.h>

struct FrameData
{
	Time deltaTime;
	EventList<struct GpuCommand>* gpuCommands;
};
