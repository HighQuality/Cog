#pragma once

class BaseFactory
{
public:
	virtual void* AllocateRawObject() = 0;
	virtual void ReturnRawObject(const void* aObject) = 0;

	virtual ~BaseFactory() = default;
};
