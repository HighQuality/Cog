#pragma once

class Awaitable
{
public:
	virtual bool IsReady() const = 0;
	virtual ~Awaitable() = default;
};
