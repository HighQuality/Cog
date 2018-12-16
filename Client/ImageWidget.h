#pragma once

class ImageWidget : public Object
{
public:
	using Base = Object;

	void Draw(RenderTarget& aRenderTarget) const override;
};
