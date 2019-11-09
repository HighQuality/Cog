#include "ClientApplicationPch.h"
#include "RenderTexture.h"

RenderTexture::RenderTexture(ID3D11Texture2D* aTexture, ID3D11RenderTargetView* aRenderTargetView,
	ID3D11ShaderResourceView* aShaderResourceView, i32 aWidth, i32 aHeight)
{
	myTexture = aTexture;
	myRenderTargetView = aRenderTargetView;
	myShaderResourceView = aShaderResourceView;
	myWidth = aWidth;
	myHeight = aHeight;
}

RenderTexture::~RenderTexture()
{
}
