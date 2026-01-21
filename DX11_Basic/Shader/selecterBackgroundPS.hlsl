#include "common.hlsl"

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

void main(in PS_INPUT input, out float4 output : SV_TARGET)
{
	float4 color = g_texture.Sample(g_sampler, input.TexCoord);
	
	output.rgb = float3(0.1, 0.1, 0.1);

	output.a = lerp(1.0f, 0.0f, step(0.1f, color.a));
	
}
