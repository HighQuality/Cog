#include "pch.h"
#include "Buffer.h"
#include "RenderEngine.h"

Buffer::Buffer(RenderEngine& aRenderer, UINT aBindFlags, D3D11_USAGE aUsage, UINT aCpuAccessFlags, UINT aMiscFlags, const void * aData, i32 aSizeInBytes)
{
	myRenderer = &aRenderer;
	myBindFlags = aBindFlags;
	myUsageFlags = aUsage;
	myCpuAccessFlags = aCpuAccessFlags;
	myMiscFlags = aMiscFlags;
	mySizeInBytes = aSizeInBytes;

	CreateBuffer(aData, aSizeInBytes);
}

Buffer::~Buffer()
{
}

void Buffer::UpdateData(const void * aData, i32 aSizeInBytes, bool aAllowResize, i32 aBufferBeginsAt)
{
	if (IsImmutable())
	{
		FATAL("Tried to update an immutable buffer");
		return;
	}

	if (aSizeInBytes > mySizeInBytes)
	{
		if (aAllowResize)
		{
			myBuffer.Reset();
			CreateBuffer(aData, aSizeInBytes);
			return;
		}
		else
		{
			FATAL(L"Data sent to buffer in UpdateData does not fit and aAllowResize is false (", aSizeInBytes, L" > ", mySizeInBytes, L")");
			return;
		}
	}

	D3D11_MAPPED_SUBRESOURCE mappedObjectResource;

	CheckDXError(
		myRenderer->GetContext()->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedObjectResource);
	);

	char* dst = static_cast<char*>(mappedObjectResource.pData);
	const char* src = static_cast<const char*>(aData);

	memcpy(dst, src + aBufferBeginsAt, aSizeInBytes - aBufferBeginsAt);
	memcpy(dst + (aSizeInBytes - aBufferBeginsAt), src, aBufferBeginsAt);

	myRenderer->GetContext()->Unmap(myBuffer.Get(), 0);
}

void Buffer::CreateBuffer(const void* aData, i32 aSizeInBytes)
{
	if (IsImmutable())
		ENSURE(aData != nullptr && aSizeInBytes > 0);

	// Fill in a buffer description.
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = myUsageFlags;
	bufferDesc.ByteWidth = static_cast<UINT>(aSizeInBytes);
	bufferDesc.BindFlags = myBindFlags;
	bufferDesc.CPUAccessFlags = myCpuAccessFlags;
	bufferDesc.MiscFlags = myMiscFlags;
	
	// Fill in the data.
	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = aData;
	initialData.SysMemPitch = 0;
	initialData.SysMemSlicePitch = 0;

	// CreateInputLayout the vertex buffer.
	CheckDXError(
		myRenderer->GetDevice()->CreateBuffer(&bufferDesc, aData ? &initialData : nullptr, &myBuffer)
	);
}
