#pragma once

class BaseFactory
{
public:
	BaseFactory() = default;

	virtual void* AllocateRawObject() = 0;
	virtual void ReturnRawObject(const void* aObject) = 0;
	virtual void ReturnAll() = 0;

	virtual ~BaseFactory() = default;
};
