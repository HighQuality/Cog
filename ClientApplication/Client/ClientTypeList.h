#pragma once
#include <Game/GameTypeList.h>

class ClientTypeList : public GameTypeList
{
public:
	using Base = GameTypeList;
	
protected:
	void RegisterTypes() override;
};
