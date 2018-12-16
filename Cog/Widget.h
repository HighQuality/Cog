#pragma once
#include "Object.h"

class CogClientGame;

class Widget : public Object
{
public:
	using Base = Object;

	Event<Widget&> OnDestroyed;

protected:
	friend CogClientGame;
	
	virtual void Tick(const Time& aDeltaTime) {  }
	virtual void Draw(RenderTarget& aRenderTarget) const {  }

	void Destroyed() override;
};

