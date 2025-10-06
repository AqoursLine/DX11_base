#include "common.hlsl"

// テクスチャ
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

void main(in PS_INPUT In, out float4 Out : SV_Target)
{
	Out = texture0.Sample(sampler0, In.TexCoord) * In.Diffuse;
	Out.a *= Material.Diffuse.a;
	
	Out *= In.Diffuse;
}
