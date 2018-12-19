#pragma once
#include "RenderingMode.h"
#include <Event.h>

#ifndef SIMULTANEOUS_RENDER_TARGETS
#define SIMULTANEOUS_RENDER_TARGETS 8
#endif

class RenderTexture;
class GenericVertexBuffer;

class RenderEngine
{
public:
	RenderEngine(HWND aWindowHandle, const RenderingMode& aMode);
	~RenderEngine();

	void Resize(i32 aWidth, i32 aHeight);

	void SetViewport(const Vec2& aTopLeft, const Vec2& aSize);
	void ResetViewport();

	void Present();
	void ClearBackbuffer();

	void SetFullscreen(bool aIsFullscreen);
	bool IsFullscreen() const;

	void ClearRenderTexture(RenderTexture& aTexture, const LinearColor& aColor);

	void Draw();

	ID3D11Device* GetDevice() const { return myDevice.Get(); }
	ID3D11DeviceContext* GetContext() const { return myContext.Get(); }

	const std::shared_ptr<RenderTexture>& GetBackbuffer() const { return myBackbuffer; }

	Event<const std::shared_ptr<RenderTexture>&> OnBackbufferRecreated;

private:
	void CreateBuffers(i32 aWidth, i32 aHeight);

	RenderingMode myRenderingMode;

	ComPtr<ID3D11Device> myDevice;
	ComPtr<ID3D11DeviceContext> myContext;
	ComPtr<IDXGISwapChain> mySwapChain;

	ComPtr<ID3D11SamplerState> mySamplerState;
	ComPtr<ID3D11BlendState> myAlphaBlendingState;
	ComPtr<ID3D11RasterizerState> myRasterState;

	ComPtr<ID3D11DepthStencilState> myDepthStencilState;

	std::shared_ptr<RenderTexture> myBackbuffer;

#if !PRODUCTION
	ComPtr<ID3D11Debug> myD3DDebug;
	ComPtr<ID3DUserDefinedAnnotation> myAnnotationWriter;
#endif
};
