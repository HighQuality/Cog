#include "pch.h"
#include <Utilities/Image.h>
#include "Texture.h"
#include "RenderEngine.h"

Texture::Texture(RenderEngine& aRenderer, const StringView& aFileName)
	: Texture(aRenderer, Image(aFileName))
{
}

Texture::Texture(RenderEngine& aRenderer, const Image& aImage)
	: Texture(aRenderer, DXGI_FORMAT_R32G32B32A32_FLOAT, aImage.ToLinearColorArray().GetData(), aImage.GetWidth(), aImage.GetHeight())
{
}

Texture::Texture(RenderEngine& aRenderer, const DXGI_FORMAT aFormat, const void* aData, const i32 aWidth, const i32 aHeight)
{
	myRenderer = &aRenderer;
	myWidth = aWidth;
	myHeight = aHeight;

	D3D11_TEXTURE2D_DESC textureDesc;

	textureDesc.Width = aWidth;
	textureDesc.Height = aHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = aFormat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.MiscFlags = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = aData;

	switch (aFormat)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		data.SysMemPitch = static_cast<UINT>(32 / 8 * aWidth);
		break;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		data.SysMemPitch = static_cast<UINT>(64 / 8 * aWidth);
		break;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		data.SysMemPitch = static_cast<UINT>(128 / 8 * aWidth);
		break;
	case DXGI_FORMAT_R32G32_FLOAT:
		data.SysMemPitch = static_cast<UINT>(64 / 8 * aWidth);
		break;
	case DXGI_FORMAT_R32_FLOAT:
		data.SysMemPitch = static_cast<UINT>(32 / 8 * aWidth);
		break;
	default:
		FATAL("Unhandled custom texture format");
	}

	data.SysMemSlicePitch = static_cast<UINT>(data.SysMemPitch * aHeight);

	CheckDXError(
		myRenderer->GetDevice()->CreateTexture2D(&textureDesc, &data, myTexture.ReleaseAndGetAddressOf())
	);

	CheckDXError(
		myRenderer->GetDevice()->CreateShaderResourceView(myTexture.Get(), nullptr, myShaderResourceView.ReleaseAndGetAddressOf());
	);
}

Texture::~Texture()
{
}

void Texture::BindToPS(int aSlot)
{
	myRenderer->GetContext()->PSSetShaderResources(aSlot, 1, myShaderResourceView.GetAddressOf());
}
