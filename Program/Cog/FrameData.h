#pragma once

struct FrameData
{
	Time deltaTime;
	EventList<struct GpuCommand>* gpuCommands;
};
