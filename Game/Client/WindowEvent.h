#pragma once

enum class WindowEventType : u8
{
	None,
	Resize,
	Close
};

struct WindowEvent
{
	WindowEventType type;

	union
	{
		struct
		{
			i32 newWidth;
			i32 newHeight;
		} resize;
	} data;
};
