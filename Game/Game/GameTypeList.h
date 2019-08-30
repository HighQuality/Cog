#pragma once
#include <Cog/EngineTypeList.h>

class GameTypeList : public EngineTypeList
{
public:
	using Base = EngineTypeList;

protected:
	void RegisterTypes() override;

};

