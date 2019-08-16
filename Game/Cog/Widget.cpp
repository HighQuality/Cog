#include "pch.h"
#include "Widget.h"

void Widget::Destroyed()
{
	OnDestroyed.Broadcast(*this);
}
