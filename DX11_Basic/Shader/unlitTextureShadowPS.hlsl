#include "common.hlsl"
#include "shadowMap.hlsl"

void main(in PS_INPUT input, out float4 outDiffuse : SV_TARGET)
{
	int2 pixelCoord = int2(input.TexCoord.x * 1024.0f, input.TexCoord.y * 1024.0f);
	float depth = g_shadowMapArray.Load(int4(pixelCoord, 0, 0)).r;

	outDiffuse = float4(depth, 0, 0, 1);
}