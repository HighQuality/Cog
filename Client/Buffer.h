#pragma once

class RenderEngine;

class Buffer
{
public:
	virtual ~Buffer();

	FORCEINLINE int GetSizeInBytes() const { return mySizeInBytes; }
	FORCEINLINE bool IsImmutable() const { return (myUsageFlags & D3D11_USAGE_IMMUTABLE) != 0; }

protected:
	Buffer(RenderEngine& aRenderer, UINT aBindFlags, D3D11_USAGE aUsage, UINT aCpuAccessFlags, UINT aMiscFlags, const void * aData, i32 aSizeInBytes);

	void UpdateData(const void * aData, i32 aSizeInBytes, bool aAllowResize = false, i32 aBufferBeginsAt = 0);

	RenderEngine* myRenderer;
	UINT myBindFlags;
	D3D11_USAGE myUsageFlags;
	UINT myCpuAccessFlags;
	UINT myMiscFlags;
	i32 mySizeInBytes;

	ComPtr<ID3D11Buffer> myBuffer;

	void CreateBuffer(const void *aData, i32 aSizeInBytes);
};

