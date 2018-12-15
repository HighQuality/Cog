#pragma once
#include <Game.h>

class CogServerGame : public Game
{
public:
	using Base = Game;

	CogServerGame();
	~CogServerGame();
	
	bool ShouldKeepRunning() const override;

protected:
	void Tick(const Time& aDeltaTime) override;

	BaseWidgetFactory& FindOrCreateWidgetFactory(const TypeID<Widget>& aWidgetType, const FunctionView<BaseWidgetFactory*()>& aFactoryCreator) override;
};

