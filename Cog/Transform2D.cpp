#include "pch.h"
#include "Transform2D.h"

void Transform2DChunkedData::SynchronizedTick()
{
	const i32 sizeBytes = GetNumObjects() * sizeof *myPreviousFramesPosition;
	memcpy(myPreviousFramesPosition, myScheduledPosition, sizeBytes);
}
