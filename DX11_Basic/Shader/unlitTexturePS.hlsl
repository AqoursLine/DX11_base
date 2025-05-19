#include "common.hlsl"

//テクスチャバッファ
Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

void main(in PS_INPUT In, out float4 outDiffuse : SV_TARGET)
{
	outDiffuse = g_texture.Sample(g_sampler, In.TexCoord);
}