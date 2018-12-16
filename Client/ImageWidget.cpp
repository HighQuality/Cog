#include "pch.h"
#include "ImageWidget.h"

void ImageWidget::Draw(RenderTarget& aRenderTarget) const
{
	Base::Draw(aRenderTarget);

	Println(L"Draw widget");
}
