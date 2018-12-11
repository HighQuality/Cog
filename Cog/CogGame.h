#pragma once

class ThreadPool;

class CogGame
{
public:
	CogGame();
	virtual ~CogGame();

	virtual bool ShouldKeepRunning() const = 0;

	virtual void Run();

protected:
	virtual void Tick(const Time& aDeltaTime) = 0;

private:
	ThreadPool* myThreadPool;
};