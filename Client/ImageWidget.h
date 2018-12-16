#pragma once
#include <Widget.h>

class ImageWidget : public Widget
{
public:
	using Base = Widget;

protected:
	void Draw(RenderTarget& aRenderTarget) const override;
};
