#pragma once
#include "BackgroundWorkAwaitable.h"

class ReadFileAwaitable : public BackgroundWorkAwaitable<Array<u8>>
{
public:
	explicit ReadFileAwaitable(const StringView& aPath);

private:
	String myPath;

	Array<u8> SynchronousWork() override;
};

