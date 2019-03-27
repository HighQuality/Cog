#pragma once
#include "Component.h"

class Transform2DChunkedData : public ComponentChunkedData
{
public:
	using Base = ComponentChunkedData;

	Transform2DChunkedData(u16 aSize)
		: Base(aSize)
	{
		myPreviousFramesPosition.Resize(aSize);
		myScheduledPosition.Resize(aSize);
	}

	void SynchronizedTick();

private:
	friend class Transform2D;

	Array<Vec2> myPreviousFramesPosition;
	Array<Vec2> myScheduledPosition;
};

class Transform2D : public InheritComponent<Component>
{
	DECLARE_CHUNKED_DATA(Transform2DChunkedData);

	DECLARE_CHUNKED_PROPERTY_ACCESSORS(PreviousFramesPosition, private, private);
	DECLARE_CHUNKED_PROPERTY_ACCESSORS(ScheduledPosition, private, private);

public:
	void SetPosition(const Vec2& aPosition) { SetScheduledPosition(aPosition); }
	const Vec2& GetPosition() const { return GetPreviousFramesPosition(); }

private:
	Ptr<Transform2D> myParentTransform;

	Vec2 myPosition;
	Vec2 myScale = Vec2::One;
	Angle myRotation;
};
