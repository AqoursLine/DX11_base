#include "common.hlsl"

//テクスチャ
Texture2D g_texture0 : register(t0);
Texture2D g_texture1 : register(t1);
SamplerState g_sampler : register(s0);

void main(in PS_INPUT In, out float4 outDiffuse : SV_Target)
{
	//テクスチャカラーを取得
	float4 color0 = g_texture0.Sample(g_sampler, In.TexCoord);
	
	float2 tex1UV = frac(In.TexCoord);

	//範囲内かをstepで判定
	float2 rangeMin = step(params3.xy, tex1UV);
	float2 rangeMax = step(tex1UV, params3.zw);
	float inRange = rangeMin.x * rangeMin.y * rangeMax.x * rangeMax.y;
	
	//相対座標を計算
	float2 relativeUV = (tex1UV - params3.xy) / (params3.zw - params3.xy);
	float2 uv1 = params2.xy + relativeUV * params2.zw;
	float4 color1 = g_texture1.Sample(g_sampler, uv1);
	
	//inRangeをマスクとして使う
	outDiffuse = lerp(color0, lerp(color0, color1, color1.a), inRange);

	outDiffuse *= In.Diffuse * Material.Diffuse;

}
