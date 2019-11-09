#include "ClientApplicationPch.h"
#include "VertexBuffer.h"
#include "RenderEngine.h"

GenericVertexBuffer::GenericVertexBuffer(RenderEngine& aRenderer, const void* aData, i32 aSizeInBytes, i32 aSingleVertexSize, bool aIsImmutable)
	: Buffer(aRenderer, D3D11_BIND_VERTEX_BUFFER, aIsImmutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC, aIsImmutable ? 0 : D3D11_CPU_ACCESS_WRITE, 0, aData, aSizeInBytes)
{
	mySingleVertexSize = aSingleVertexSize;
}

GenericVertexBuffer::~GenericVertexBuffer()
{
}

void GenericVertexBuffer::Bind(i32 aSlot) const
{
	UINT vertexSize = static_cast<UINT>(GetSingleVertexSize());
	UINT offset = 0;
	myRenderer->GetContext()->IASetVertexBuffers(aSlot, 1, myBuffer.GetAddressOf(), &vertexSize, &offset);
}
