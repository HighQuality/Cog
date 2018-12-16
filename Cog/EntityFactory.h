#pragma once
#include "Factory.h"
#include "Entity.h"

class EntityFactoryChunk : public FactoryChunk<Entity>
{
public:
	using Base = FactoryChunk<Entity>;

	EntityFactoryChunk(const u16 aSize)
		: Base(aSize)
	{
		myGeneration.Resize(aSize);

		for (u16& generation : myGeneration)
			generation = 1;

	}

	Entity& Allocate() override
	{
		Entity& entity = Base::Allocate();
		entity.myChunk = this;
		myGeneration[this->IndexOf(entity)]++;
		return entity;
	}

	void Return(const Entity& aObject) override
	{
		const u16 index = this->IndexOf(aObject);

		Base::Return(aObject);

		myGeneration[index]++;
	}

	FORCEINLINE u16 FindGeneration(const Entity& aEntity) const
	{
		return myGeneration[this->IndexOf(aEntity)];
	}

private:
	Array<u16> myGeneration;
};

class EntityFactory : public Factory<Entity, EntityFactoryChunk>
{
};
