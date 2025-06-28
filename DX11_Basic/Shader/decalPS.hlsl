#include "common.hlsl"

Texture2D g_DecalTexture : register(t0);
SamplerState g_DecalSampler : register(s0);

struct DecalPS_INPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : POSITION0;
	float2 TexCoord : TEXCOORD0;
	float4 Params : PARAMS;
};

void main(in DecalPS_INPUT In, out float4 Out : SV_TARGET)
{
	//デカールテクスチャをサンプリング
	float4 decalColor = g_DecalTexture.Sample(g_DecalSampler, In.TexCoord);
	
	//中心からの距離を計算
	float2 center = In.TexCoord - 0.5f;
	float distance = length(center);
	
	//円形のフェードアウト
}