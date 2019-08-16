#pragma once
#include <Cog/EngineComponentList.h>

class GameComponentList : public EngineComponentList
{
public:
	using Base = EngineComponentList;

protected:
	void RegisterComponents() override;

};

