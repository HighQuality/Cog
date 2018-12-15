#pragma once

class ImageWidget : public Widget
{
public:
	using Base = Widget;

	void Draw(RenderTarget& aRenderTarget) const override;
};
