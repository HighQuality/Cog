#pragma once
#include <CogScene.h>

class Scene : public CogScene
{
public:
	Scene(CogGame& aGame);
	~Scene();

protected:
	Scene* GetSubPointer() final { return this; }
	const Scene* GetSubPointer() const final { return this;  }
};

