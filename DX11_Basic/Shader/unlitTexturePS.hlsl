#include "common.hlsl"

//テクスチャバッファ
Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

void main(in PS_INPUT In, out float4 outDiffuse : SV_TARGET)
{

	if (Material.TextureEnable == 1)
	{
		//テクスチャカラー取得
		outDiffuse = g_texture.Sample(g_sampler, In.TexCoord);
	}
	else
	{
		//テクスチャ無効時は白色
		outDiffuse = float4(1, 1, 1, 1);
	}

	//マテリアルのベースカラーを乗算
	outDiffuse *= Material.Diffuse * In.Diffuse;
}