#pragma once

class Texture;
class GenericVertexBuffer;

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
};
