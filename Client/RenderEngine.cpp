#include "pch.h"
#include "RenderEngine.h"
#include "RenderTexture.h"

void SetResourceDebugName(ID3D11DeviceChild* resource, StringView aDebugName)
{
	if (!PRODUCTION && resource)
	{
		const std::string ascii = aDebugName.ToStdString();
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(ascii.length()), ascii.data());
	}
}

RenderEngine::RenderEngine(HWND aWindowHandle, const RenderingMode& aMode)
{
	myRenderingMode = aMode;

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	if constexpr (!PRODUCTION)
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
	swapChainDesc.Flags = 0;
	swapChainDesc.BufferDesc.Width = CastBoundsChecked<UINT>(aMode.width);
	swapChainDesc.BufferDesc.Height = CastBoundsChecked<UINT>(aMode.height);
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferCount = 2;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	swapChainDesc.OutputWindow = aWindowHandle;
	swapChainDesc.Windowed = !aMode.fullscreen;
	// TODO: Add support for DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING (https://blogs.msdn.microsoft.com/directx/2018/04/09/dxgi-flip-model/ etc)
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	// TODO: Fallback to DXGI_SWAP_EFFECT_DISCARD if DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING is not present
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	D3D_FEATURE_LEVEL featureLevel;
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	CheckDXError(
		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
			&featureLevel, 1,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			mySwapChain.ReleaseAndGetAddressOf(),
			myDevice.ReleaseAndGetAddressOf(),
			nullptr,
			myContext.ReleaseAndGetAddressOf())
	);

	// Disables Alt+Enter in non-development
	if constexpr (PRODUCTION)
	{
		ComPtr<IDXGIDevice> dxgiDevice;

		bool successfullyDisabledAltEnter = false;

		if (SUCCEEDED(myDevice.As(&dxgiDevice)))
		{
			ComPtr<IDXGIAdapter> dxgiAdapter;

			if (SUCCEEDED(dxgiDevice.As(&dxgiAdapter)))
			{
				ComPtr<IDXGIFactory> pIDXGIFactory = nullptr;
				// TODO: Rewrite to dxgiAdapter.As equivalent for parents
				dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &pIDXGIFactory);

				if (dxgiAdapter != nullptr)
				{
					if (SUCCEEDED(pIDXGIFactory->MakeWindowAssociation(aWindowHandle, DXGI_MWA_NO_ALT_ENTER)))
						successfullyDisabledAltEnter = true;
				}
			}
		}

		ENSURE(successfullyDisabledAltEnter);
	}

	CheckDXError(
		myContext.As(&myAnnotationWriter)
	);

	UINT levels;
	myDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &levels);

	myD3DDebug = nullptr;

#ifdef _DEBUG

	if (SUCCEEDED(myDevice.As(&myD3DDebug)))
	{
		ComPtr<ID3D11InfoQueue> d3dInfoQueue = nullptr;
		if (SUCCEEDED(myD3DDebug.As(&d3dInfoQueue)))
		{
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);

			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS
			};

			D3D11_INFO_QUEUE_FILTER filter;
			memset(&filter, 0, sizeof(filter));
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
		}
	}
#endif

	CreateBuffers(aMode.width, aMode.height);

	D3D11_RASTERIZER_DESC rasterDesc;
	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;

	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// CreateInputLayout the rasterizer state from the description we just filled out.
	CheckDXError(
		myDevice->CreateRasterizerState(&rasterDesc, myRasterState.ReleaseAndGetAddressOf())
	);

	myContext->RSSetState(myRasterState.Get());

	D3D11_BLEND_DESC blendStateDescription;
	// Clear the blend state description.
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
	// CreateInputLayout an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	for (size_t i = 1; i < 8; ++i)
	{
		memcpy(&blendStateDescription.RenderTarget[i], &blendStateDescription.RenderTarget[0],
		       sizeof blendStateDescription.RenderTarget[0]);
	}

	blendStateDescription.IndependentBlendEnable = true;

	CheckDXError(
		myDevice->CreateBlendState(&blendStateDescription, myAlphaBlendingState.ReleaseAndGetAddressOf())
	);
	
	// Turn on the alpha blending.
	myContext->OMSetBlendState(myAlphaBlendingState.Get(), nullptr, 0xffffffff);

	// Create sampler
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	CheckDXError(
		myDevice->CreateSamplerState(&samplerDesc, mySamplerState.ReleaseAndGetAddressOf())
	);
	
	myContext->PSSetSamplers(0, 1, mySamplerState.GetAddressOf());
	myContext->VSSetSamplers(0, 1, mySamplerState.GetAddressOf());
	myContext->GSSetSamplers(0, 1, mySamplerState.GetAddressOf());

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc { };

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	D3D11_DEPTH_STENCIL_DESC disableReadDesc = depthStencilDesc;

	CheckDXError(
		myDevice->CreateDepthStencilState(&depthStencilDesc, myDepthStencilState.ReleaseAndGetAddressOf())
	);

	myContext->OMSetDepthStencilState(myDepthStencilState.Get(), 0);

	myContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

RenderEngine::~RenderEngine()
{
	if (IsFullscreen())
	{
		SetFullscreen(false);
	}
}

void RenderEngine::Resize(i32 aWidth, i32 aHeight)
{
	if (!ENSURE(mySwapChain))
		return;

	Println(L"Resizing back buffer to ", aWidth, L"x", aHeight);

	ID3D11RenderTargetView* renderTargets = nullptr;
	myContext->OMSetRenderTargets(1, &renderTargets, nullptr);

	myBackbuffer = nullptr;

	CheckDXError(
		mySwapChain->ResizeBuffers(0, aWidth, aHeight, DXGI_FORMAT_UNKNOWN, 0)
	);

	CreateBuffers(aWidth, aHeight);
}

void RenderEngine::Present()
{
	mySwapChain->Present(myRenderingMode.vsync ? 1 : 0, 0);

	myContext->OMSetRenderTargets(1, myBackbuffer->myRenderTargetView.GetAddressOf(), nullptr);
}

void RenderEngine::SetViewport(const Vec2& aTopLeft, const Vec2& aSize)
{
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = aTopLeft.x;
	viewport.TopLeftY = aTopLeft.y;
	viewport.Width = aSize.x;
	viewport.Height = aSize.y;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	myContext->RSSetViewports(1, &viewport);
}

void RenderEngine::ResetViewport()
{
	SetViewport(Vec2::Zero, Vec2::One * Vec2(static_cast<f32>(myBackbuffer->GetWidth()), static_cast<f32>(myBackbuffer->GetHeight())));
}

void RenderEngine::ClearBackbuffer()
{
	if (!ENSURE(myBackbuffer))
		return;

	ClearRenderTexture(*myBackbuffer, LinearColor::CornflowerBlue);
}

void RenderEngine::ClearRenderTexture(RenderTexture& aTexture, const LinearColor& aColor)
{
	myContext->ClearRenderTargetView(aTexture.myRenderTargetView.Get(), aColor.GetData());
}

void RenderEngine::Draw()
{
	myContext->Draw(6, 0);
}

void RenderEngine::CreateBuffers(const i32 aWidth, const i32 aHeight)
{
	myRenderingMode.width = aWidth;
	myRenderingMode.height = aHeight;

	ComPtr<ID3D11Texture2D> texture = nullptr;

	// TODO: Rewrite to .As equivalent(?)
	CheckDXError(
		mySwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &texture)
	);

	ComPtr<ID3D11RenderTargetView> renderTargetView;
	CheckDXError(
		myDevice->CreateRenderTargetView(texture.Get(), NULL, &renderTargetView)
	);

	ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	CheckDXError(
		myDevice->CreateShaderResourceView(texture.Get(), nullptr, &shaderResourceView)
	);

	SetResourceDebugName(texture.Get(), L"Backbuffer texture");
	SetResourceDebugName(renderTargetView.Get(), L"Backbuffer rendertargetview");
	SetResourceDebugName(shaderResourceView.Get(), L"Backbuffer shader resource view");

	myBackbuffer = std::make_shared<RenderTexture>(texture.Get(), renderTargetView.Get(), shaderResourceView.Get(), aWidth, aHeight);

	myContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);
	ResetViewport();
}

void RenderEngine::SetFullscreen(const bool aIsFullscreen)
{
	mySwapChain->SetFullscreenState(aIsFullscreen, nullptr);
}

bool RenderEngine::IsFullscreen() const
{
	BOOL isFullscreen;
	mySwapChain->GetFullscreenState(&isFullscreen, nullptr);
	return isFullscreen;
}
