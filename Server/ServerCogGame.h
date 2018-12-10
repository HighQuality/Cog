#pragma once
#include <CogGame.h>

class ServerCogGame : public CogGame
{
public:
	ServerCogGame();
	~ServerCogGame();
	
	bool ShouldKeepRunning() const override;

protected:
	void Tick(const Time& aDeltaTime) override;
};

