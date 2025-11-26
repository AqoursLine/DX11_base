#include "common.hlsl"

void main(in PS_INPUT In, out float4 outDiffuse : SV_Target)
{
	float4 baseColor = In.Diffuse * Material.Diffuse;

	float4 normal = normalize(In.Normal);
	float3 eyev = normalize(CameraPosition.xyz - In.WorldPosition.xyz);

	float3 finalLight = Material.Ambient.rgb;
	float3 finalSpecular = 0.0f;
	
	// ディレクションライトのループ
	for (uint i = 0; i < DirectionalLightCount; i++)
	{
		// 拡散光
		float3 lightDir = -normalize(Lights[i].DirectionAndIntensity.xyz);
		float NdotL = max(dot(normal.xyz, lightDir), 0.0f);
		finalLight += Lights[i].DiffuseAndRange.rgb * NdotL * Lights[i].DirectionAndIntensity.w;

		// スペキュラー反射(Blinn-Phongモデル)
		float3 halfVec = normalize(lightDir + eyev);
		float specularIntensity = pow(max(dot(normal.xyz, halfVec), 0.0f), Material.Shininess);
		finalSpecular += specularIntensity * Lights[i].DirectionAndIntensity.w * Material.Specular.rgb;
	}


	finalLight = saturate(finalLight);

	outDiffuse.rgb = baseColor.rgb * finalLight;
	outDiffuse.rgb += finalSpecular;

	outDiffuse.a = 1.0f;
}
