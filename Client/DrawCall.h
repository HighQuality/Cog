#pragma once

class Texture;
class GenericVertexBuffer;
class DrawCallList;

constexpr size MaxPixelShaderTextures = 4;

class DrawCall
{
public:
	DrawCall();
	~DrawCall();

	DrawCall(DrawCall&&);
	DrawCall(const DrawCall&);

	DrawCall& operator=(DrawCall&&);
	DrawCall& operator=(const DrawCall&);
	 
	std::shared_ptr<GenericVertexBuffer> vertexBuffer;
	std::shared_ptr<GenericVertexBuffer> instanceBuffer;

	std::array<std::shared_ptr<Texture>, MaxPixelShaderTextures> pixelShaderTextures;

	void Submit();

private:
	static thread_local DrawCallList ourLocalDrawCallList;
};

class DrawCallList : public Array<DrawCall>
{
public:
	DrawCallList()
	{
		std::unique_lock<std::mutex> lck(ourInstancesMutex);
		ourInstances.Add(this);
	}

	~DrawCallList()
	{
		std::unique_lock<std::mutex> lck(ourInstancesMutex);
		ourInstances.RemoveSwap(this, true);
	}

	static Array<DrawCall> GatherDrawCalls();

private:
	static Array<DrawCallList*> ourInstances;
	static std::mutex ourInstancesMutex;
};
