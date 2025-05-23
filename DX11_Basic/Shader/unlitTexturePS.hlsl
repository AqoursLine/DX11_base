#include "common.hlsl"

//テクスチャバッファ
Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

void main(in PS_INPUT In, out float4 outDiffuse : SV_TARGET)
{
	float flag = step(0.5f, Material.TextureEnable);
	float4 texColor = g_texture.Sample(g_sampler, In.TexCoord);
	
	outDiffuse = lerp(In.Diffuse, texColor, flag);
}