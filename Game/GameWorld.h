#pragma once
#include <CogGameWorld.h>

class GameWorld : public CogGameWorld
{
public:
	GameWorld();
	~GameWorld();

protected:
	GameWorld* GetSubPointer() final { return this; }
	const GameWorld* GetSubPointer() const final { return this;  }
};

