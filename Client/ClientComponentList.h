#pragma once
#include <GameComponentList.h>

class ClientComponentList : public GameComponentList
{
public:
	using Base = GameComponentList;
	
protected:
	void RegisterComponents() override;
};
