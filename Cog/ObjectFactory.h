#pragma once
#include "Factory.h"
#include "Object.h"

class ObjectFactoryChunk : public FactoryChunk<Object>
{
public:
	using Base = FactoryChunk<Object>;

	ObjectFactoryChunk(const u16 aSize)
		: Base(aSize)
	{
		myGeneration.Resize(aSize);

		for (u16& generation : myGeneration)
			generation = 1;

	}

	Object& Allocate() override
	{
		Object& object = Base::Allocate();
		object.myChunk = this;
		myGeneration[this->IndexOf(object)]++;
		return object;
	}

	void Return(const Object& aObject) override
	{
		const u16 index = this->IndexOf(aObject);

		Base::Return(aObject);

		myGeneration[index]++;
	}

	FORCEINLINE u16 FindGeneration(const Object& aObject) const
	{
		return myGeneration[this->IndexOf(aObject)];
	}

private:
	Array<u16> myGeneration;
};

class ObjectFactory : public Factory<Object, ObjectFactoryChunk>
{
};
