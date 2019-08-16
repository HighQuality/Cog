#pragma once

enum class GpuCommandType
{
	Unknown,
	DrawSprite
};

struct GpuCommand
{
	GpuCommandType type;

	union
	{
		struct
		{
			class SpriteComponent* sprite;
		} drawSpriteData;
	};
};
