#include "common.hlsl"
//テクスチャ設定
Texture2D g_Texture : register(t0);

//サンプラーステート
SamplerState g_SamplerState : register(s0);

void main(in PS_INPUT In, out float4 outDiffuse : SV_TARGET)
{
	//ピクセル法線を正規化
	float4 normal = normalize(In.Normal);

	//光源処理
	float light = -dot(normal.xyz, Light.Direction.xyz);
	light = saturate(light);

	//テクスチャのピクセル色を取得
	if (Material.TextureEnable)
	{
		outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);		
	}else
	{
		outDiffuse = Material.Diffuse;
	}
	outDiffuse.rgb *= light;
	outDiffuse.a = In.Diffuse.a;
	
	//視線ベクトル作成
	float3 eyev = In.WorldPosition - CameraPosition;
	eyev = normalize(eyev);
	
	//光の反射ベクトル
	float3 refv = reflect(Light.Direction.xyz, normal.xyz);
	refv = normalize(refv);
	
	//反射ベクトルと視線ベクトルの角度計算
	float specular = -dot(refv, eyev);
	specular = saturate(specular);
	specular = pow(specular, 30);

	outDiffuse.rgb += specular;
	
	//環境光を加算
	outDiffuse.rgb += Light.Ambient.rgb;
}