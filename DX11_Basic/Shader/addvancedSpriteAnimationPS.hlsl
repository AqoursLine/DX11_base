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

	//範囲内か
	if (tex1UV.x >= params3.x && tex1UV.x <= params3.z &&
		tex1UV.y >= params3.y && tex1UV.y <= params3.w)
	{
		//範囲内での相対座標を計算
		float2 relativeUV = (tex1UV - params3.xy) / (params3.zw - params3.xy);
		
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
