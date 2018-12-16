#pragma once

class RenderTarget;
class Object;

class BaseObjectFactoryChunk
{
public:
	BaseObjectFactoryChunk(const u16 aSize)
	{
		myGeneration.Resize(aSize);

		for (u16& generation : myGeneration)
			generation = 1;
	}

	virtual ~BaseObjectFactoryChunk() = default;

	virtual void ReturnByID(u16 aIndex) = 0;

	FORCEINLINE u16 FindGeneration(const u16 aIndex) const
	{
		return myGeneration[aIndex];
	}

protected:
	void InitializeSOAProperties(const u16 aIndex)
	{
		myGeneration[aIndex]++;
	}

	void DestroySOAProperties(const u16 aIndex)
	{
		myGeneration[aIndex]++;
	}

	friend Object;
	
private:
	Array<u16> myGeneration;
};
