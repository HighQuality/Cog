#pragma once
#include "FactoryChunk.h"
#include "BaseObjectFactoryChunk.h"

template <typename T>
class ObjectFactoryChunk final : public FactoryChunk<T>, public BaseObjectFactoryChunk
{
public:
	using Base = FactoryChunk<T>;

	ObjectFactoryChunk(const u16 aSize)
		: Base(aSize), BaseObjectFactoryChunk(aSize)
	{
	}

	T& Allocate() override
	{
		T& widget = Base::Allocate();
		widget.myChunk = this;
		widget.myChunkIndex = this->IndexOf(widget);

		InitializeSOAProperties(widget.myChunkIndex);

		return widget;
	}

	void Return(const T& aObject) override
	{
		const u16 index = this->IndexOf(aObject);

		Base::Return(aObject);

		DestroySOAProperties(index);
	}

	void ReturnByID(u16 aIndex) override
	{
		Return(this->myObjectsData[aIndex]);
	}
};
