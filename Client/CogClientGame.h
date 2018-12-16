#pragma once
#include <Game.h>

class Window;
class RenderEngine;
class BaseWidgetFactory;

class CogClientGame : public Game
{
public:
	using Base = Game;

	CogClientGame();
	~CogClientGame();

	bool ShouldKeepRunning() const override;

	void Run() override;

protected:
	void Tick(const Time& aDeltaTime) override;

	void DispatchTick(const Time& aDeltaTime) override;
	void DispatchDraw(RenderTarget& aRenderTarget);

	BaseWidgetFactory& FindOrCreateWidgetFactory(const TypeID<Widget>& aWidgetType, const FunctionView<BaseWidgetFactory*()>& aFactoryCreator) final;

	void DispatchWork(const Time& aDeltaTime) override;

private:
	Object& CreateCamera();

	Array<BaseWidgetFactory*> myWidgetFactories;

	Window* myWindow = nullptr;
	RenderEngine* myRenderer = nullptr;

	Ptr<Object> myCamera;
};

