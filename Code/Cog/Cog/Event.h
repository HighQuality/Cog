#pragma once

class Event
{
public:
	static constexpr i32 InlinedSize = 128 - sizeof(void*);
};
