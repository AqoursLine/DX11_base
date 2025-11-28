#include "common.hlsl"
#include "shadowMap.hlsl"

SamplerState g_sampler : register(s0);

void main(in PS_INPUT input, out float4 outDiffuse : SV_TARGET)
{
	float4 depth = g_shadowMapArray.Sample(g_sampler, float3(input.TexCoord, 0));

	outDiffuse = depth;
	outDiffuse.a = 1.0f;
}