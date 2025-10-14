#include "common.hlsl"

//テクスチャ
Texture2D g_texture0 : register(t0);
Texture2D g_texture1 : register(t1);
SamplerState g_sampler : register(s0);

void main(in PS_INPUT In, out float4 outDiffuse : SV_Target)
{
	//params1.xyを繰り返し対応
	float2 param1 = frac(params1.xy);

	//テクスチャカラーを取得
	float2 uv0 = param1.xy + In.TexCoord * params1.zw;
	float4 color0 = g_texture0.Sample(g_sampler, uv0);
	
	//params3の範囲内か
	float2 rectMin = frac(params3.xy - param1.xy);
	float2 rectMax = frac(params3.zw - param1.xy);

	//範囲内か
	if (In.TexCoord.x >= rectMin.x && In.TexCoord.x <= rectMax.x &&
		In.TexCoord.y >= rectMin.y && In.TexCoord.y <= rectMax.y)
	{
		//範囲内での相対座標を計算
		float2 relativeUV = (In.TexCoord - rectMin) / (rectMax - rectMin);
		
		//テクスチャ1のUVを計算
		float2 uv1 = params2.xy + relativeUV * params2.zw;
		float4 color1 = g_texture1.Sample(g_sampler, uv1);
		
		//合成
		outDiffuse = lerp(color0, color1, color1.a);

	}
	else
	{
		outDiffuse = color0;
	}

	outDiffuse *= In.Diffuse * Material.Diffuse;

}
