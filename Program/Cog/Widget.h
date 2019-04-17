#pragma once
#include "Object.h"
#include "Event.h"

class CogClientGame;

class Widget : public Object
{
public:
	using Base = Object;

	Event<Widget&> OnDestroyed;

protected:
	friend CogClientGame;
	
	virtual void Tick(const FrameData& aFrameData) {  }

	void Destroyed() override;
};

