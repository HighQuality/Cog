#pragma once

class RenderTexture
{
public:
	RenderTexture(ID3D11Texture2D* aTexture, ID3D11RenderTargetView* aRenderTargetView, ID3D11ShaderResourceView* aShaderResourceView, i32 aWidth, i32 aHeight);
	~RenderTexture();

	i32 GetWidth() const { return myWidth; }
	i32 GetHeight() const { return myHeight; }

private:
	friend class RenderEngine;
	
	i32 myWidth;
	i32 myHeight;

	ComPtr<ID3D11Texture2D> myTexture;
	ComPtr<ID3D11RenderTargetView> myRenderTargetView;
	ComPtr<ID3D11ShaderResourceView> myShaderResourceView;
};
