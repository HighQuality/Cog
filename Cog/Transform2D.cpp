#include "pch.h"
#include "Transform2D.h"

void Transform2DChunkedData::SynchronizedTick()
{
	const i32 sizeBytes = myPreviousFramesPosition.GetLength() * sizeof myPreviousFramesPosition[0];
	memcpy(myPreviousFramesPosition.GetData(), myScheduledPosition.GetData(), sizeBytes);
}
