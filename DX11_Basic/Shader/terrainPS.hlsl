#include "common.hlsl"

Texture2D<float4> g_normalMap : register(t1);
SamplerState g_normalSampler : register(s0);

void main (in PS_INPUT In, out float4 Out : SV_Target)
{
	//ノーマルマップから法線をサンプリング
	float4 normalMapSample = g_normalMap.Sample(g_normalSampler, In.TexCoord);
	float3 normal = normalize(normalMapSample.xyz * 2.0f - 1.0f);
	
	//法線がゼロベクトルの場合、デフォルトの法線を使用
	if (length(normal) < 0.001f)
	{
		normal = In.Normal;
	}
	
	//ライティング計算
	float3 lightDir = normalize(-Light.Direction.xyz);
	float diffuseIntensity = max(dot(normal, lightDir), 0.0f);
	
	//地形の基本色(緑色の草地)
	float3 terrainColor = float3(0.2f, 0.6f, 0.2f);
	
	//最終色の計算
	float3 ambient = Light.Ambient.rgb * Material.Ambient.rgb * terrainColor;
	float3 diffuse = Light.Diffuse.rgb * Material.Diffuse.rgb * diffuseIntensity * terrainColor;
	
	float3 finalColor = ambient + diffuse;
	
	Out = float4(finalColor, 1.0f);
}
