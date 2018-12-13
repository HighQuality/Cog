#pragma once
#include <GameComponentList.h>

class ServerComponentList : public GameComponentList
{
public:
	using Base = GameComponentList;

protected:
	void RegisterComponents() override;
};

