#include "pch.h"
#include "Widget.h"

Widget::Widget()
{
}

Widget::~Widget()
{
	myChunk = nullptr;
	myChunkIndex = 0;
}

void Widget::Tick(const Time& aDeltaTime)
{
}

void Widget::Draw(RenderTarget& aRenderTarget) const
{
}

void Widget::Destroy()
{
	OnDestroyed.Broadcast(*this);
	myChunk->ReturnByID(myChunkIndex);
}
