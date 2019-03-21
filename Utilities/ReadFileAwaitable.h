#pragma once
#include "BackgroundWorkAwaitable.h"

class ReadFileAwaitable : public BackgroundWorkAwaitable
{
public:
	explicit ReadFileAwaitable(const StringView& aPath);

private:
	String myPath;

	void SynchronousWork() override;
};

