#include "common.hlsl"
//テクスチャ設定
Texture2D g_Texture : register(t0);

//サンプラーステート
SamplerState g_SamplerState : register(s0);

void main(in PS_INPUT In, out float4 outDiffuse : SV_TARGET)
{
	//ピクセル法線を正規化
	float4 normal = normalize(In.Normal);
	
	//テクスチャの色を取得
	float4 texColor = g_Texture.Sample(g_SamplerState, In.TexCoord);
	
	//マテリアルの拡散色を計算
	float4 baseColor = lerp(Material.Diffuse * In.Diffuse, texColor * Material.Diffuse * In.Diffuse, (float)Material.TextureEnable);

	//環境光の初期化
	float3 finalLight = Material.Ambient.rgb;
	float3 finalSpecular = 0.0f;
	
	//視線ベクトル
	float3 eyev = In.WorldPosition.xyz - CameraPosition.xyz;
	eyev = normalize(eyev);
	
	//各ライトについてループ
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		//拡散光
		float lightIntensity = -dot(normal.xyz, Lights[i].DirectionAndIntensity.xyz);
		//クランプ
		lightIntensity = saturate(lightIntensity);

		//拡散反射光の影響を合計
		finalLight += lightIntensity * Lights[i].DiffuseAndRange.rgb * Lights[i].DirectionAndIntensity.w;
		
		//スペキュラー反射
		float3 refv = reflect(Lights[i].DirectionAndIntensity.xyz, normal.xyz);
		refv = normalize(refv);
		
		//視線ベクトルと反射ベクトルの内積
		float specularIntensity = -dot(refv, eyev);
		specularIntensity = saturate(specularIntensity);
		//スペキュラー反射光の影響を合計
		specularIntensity = pow(specularIntensity, Material.Shininess) * Lights[i].DirectionAndIntensity.w;
		
		//鏡面反射の色
		finalSpecular += specularIntensity * Material.Specular.rgb;
	}
	
	//最終的な拡散色をクランプ
	finalLight = saturate(finalLight);
	
	//拡散色と鏡面反射色を合成
	outDiffuse.rgb = baseColor.rgb * finalLight;
	outDiffuse.rgb += finalSpecular;

	outDiffuse.a = baseColor.a;
}