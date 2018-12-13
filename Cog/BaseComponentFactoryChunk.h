#pragma once

class RenderTarget;
class Object;
class Component;

class BaseComponentFactoryChunk
{
public:
	BaseComponentFactoryChunk(const u16 aSize)
	{
		myObjects.Resize(aSize);
		myGeneration.Resize(aSize);
		myReceiveTicks.Resize(aSize);
		myIsVisible.Resize(aSize);

		for (auto& object : myObjects)
			object = nullptr;

		for (bool& tick : myReceiveTicks)
			tick = false;

		for (bool& visible : myIsVisible)
			visible = false;

		for (u16& generation : myGeneration)
			generation = 1;
	}

	virtual ~BaseComponentFactoryChunk() = default;

	virtual void DispatchTick(Time aDeltaTime) = 0;
	virtual void DispatchDraw2D(RenderTarget& aRenderTarget) = 0;
	virtual void DispatchDraw3D(RenderTarget& aRenderTarget) = 0;

	virtual void ReturnByID(u16 aIndex) = 0;

	FORCEINLINE u16 FindGeneration(const u16 aIndex) const
	{
		return myGeneration[aIndex];
	}

	FORCEINLINE Object& FindObject(const u16 aIndex) const
	{
		return *myObjects[aIndex];
	}
	
protected:
	void InitializeSOAProperties(const u16 aIndex)
	{
		myObjects[aIndex] = nullptr;
		myGeneration[aIndex]++;
		myReceiveTicks[aIndex] = true;
		myIsVisible[aIndex] = true;
	}

	void DestroySOAProperties(const u16 aIndex)
	{
		myObjects[aIndex] = nullptr;
		myGeneration[aIndex]++;
		myReceiveTicks[aIndex] = false;
		myIsVisible[aIndex] = false;
	}

	friend Component;
	friend Object;
	
	FORCEINLINE void AssignObject(const u16 aIndex, Object& aObject)
	{
		myObjects[aIndex] = &aObject;
	}

	FORCEINLINE void SetTickEnabled(const u16 aIndex, const bool aTickEnabled)
	{
		myReceiveTicks[aIndex] = aTickEnabled;
	}

	FORCEINLINE bool IsTickEnabled(const u16 aIndex) const
	{
		return myReceiveTicks[aIndex];
	}

	FORCEINLINE void SetIsVisible(const u16 aIndex, const bool aIsVisible)
	{
		myIsVisible[aIndex] = aIsVisible;
	}

	FORCEINLINE bool IsVisible(const u16 aIndex) const
	{
		return myIsVisible[aIndex];
	}

private:
	Array<Object*> myObjects;
	Array<u16> myGeneration;
	Array<bool> myReceiveTicks;
	Array<bool> myIsVisible;
};
