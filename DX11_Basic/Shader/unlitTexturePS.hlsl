#include "common.hlsl"

//テクスチャバッファ
Texture2D g_Texture : register(t0);

SamplerState g_Sampler : register(s0);

void main(in PS_INPUT In, out float4 outDiffuse : SV_TARGET)
{
	outDiffuse = g_Texture.Sample(g_Sampler, In.TexCoord);
	
	//色を掛け算
	outDiffuse *= In.Diffuse;
}