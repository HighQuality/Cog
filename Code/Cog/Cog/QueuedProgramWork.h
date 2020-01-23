#pragma once

struct QueuedProgramWork
{
	void(*function)(void*) = nullptr;
	void* argument = nullptr;
};
