#pragma once
#include "Buffer.h"

class GenericVertexBuffer : public Buffer
{
public:
	GenericVertexBuffer(RenderEngine& aRenderer, const void* aData, i32 aSizeInBytes, i32 aSingleVertexSize, bool aIsImmutable = true);
	~GenericVertexBuffer();

	FORCEINLINE i32 GetSingleVertexSize() const { return mySingleVertexSize; }
	FORCEINLINE i32 GetCount() const { return GetSizeInBytes() / GetSingleVertexSize(); }

	void Bind(i32 aSlot) const;

private:
	i32 mySingleVertexSize;
};

template<typename TVertex>
class VertexBuffer : public GenericVertexBuffer
{
public:
	VertexBuffer(RenderEngine& aRenderer)
		: VertexBuffer(aRenderer, ArrayView<TVertex>(), false)
	{
	}

	VertexBuffer(RenderEngine& aRenderer, const ArrayView<TVertex>& aData, bool aIsImmutable = true)
		: GenericVertexBuffer(aRenderer, aData.GetData(), sizeof(TVertex) * aData.GetLength(), sizeof(TVertex), aIsImmutable)
	{
	}

	void UpdateData(const ArrayView<TVertex>& aData, bool aAllowResize = false, int aBufferBeginsAt = 0)
	{
		GenericVertexBuffer::UpdateData(aData.GetData(), sizeof(TVertex) * aData.GetLength(), aAllowResize, aBufferBeginsAt * sizeof(TVertex));
	}
};
