#pragma once

class Image;
class RenderEngine;

class Texture
{
public:
	Texture(RenderEngine& aRenderer, const StringView& aFileName);
	Texture(RenderEngine& aRenderer, const Image& aImage);
	Texture(RenderEngine& aRenderer, DXGI_FORMAT aFormat, const void* aData, i32 aWidth, i32 aHeight);
	~Texture();

	void BindToPS(int aSlot);

private:
	RenderEngine* myRenderer;

	i32 myWidth;
	i32 myHeight;

	ComPtr<ID3D11Texture2D> myTexture;
	ComPtr<ID3D11ShaderResourceView> myShaderResourceView;
};

