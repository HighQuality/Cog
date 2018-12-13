#pragma once
#include "ComponentList.h"

class EngineComponentList : public ComponentList
{
public:
	using Base = ComponentList;

protected:
	void RegisterComponents() override;
};

