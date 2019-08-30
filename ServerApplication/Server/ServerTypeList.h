#pragma once
#include <Game/GameTypeList.h>

class ServerTypeList : public GameTypeList
{
public:
	using Base = GameTypeList;

protected:
	void RegisterTypes() override;
};

