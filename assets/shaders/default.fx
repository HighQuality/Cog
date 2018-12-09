
struct VertexInputType
{
    float4 position : POSITION;
	float4 color : INSTANCE_COLOR;
	float2 uv : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct PixelOutputType
{
	float4 color : SV_TARGET;
};


PixelInputType vs_main(VertexInputType input)
{
    PixelInputType output;

	output.position = input.position;
	output.color = input.color;
	output.uv = input.uv;

    return output;
}

PixelOutputType ps_main(PixelInputType input)
{
	PixelOutputType output;
	output.color = input.color;
	return output;
}
