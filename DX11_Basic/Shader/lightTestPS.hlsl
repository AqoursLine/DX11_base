#include "common.hlsl"
#include "shadowMap.hlsl"

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

void main(in PS_INPUT In, out float4 outDiffuse : SV_Target)
{
	float4 baseColor = In.Diffuse * Material.Diffuse;

	if (Material.TextureEnable)
	{
		baseColor *= g_texture.Sample(g_sampler, In.TexCoord);
	}

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

	// 点光源のループ
	for (uint j = DirectionalLightCount; j < DirectionalLightCount + PointLightCount; j++)
	{
		// ライトベクトルと距離の計算
		float3 lightVec = Lights[j].PositionAndType.xyz - In.WorldPosition.xyz;
		float distance = length(lightVec);
		float3 lightDir = lightVec / distance;
		
		// 減衰係数
		float attenuation = 1.0f / (
			Lights[j].attenuation.x + 
			Lights[j].attenuation.y * distance + 
			Lights[j].attenuation.z * distance * distance
		);
		
		float rangeFactor = saturate(1.0f - distance / Lights[j].DiffuseAndRange.w);
		
		attenuation *= rangeFactor;

		// 拡散光
		float NdotL = max(dot(normal.xyz, lightDir), 0.0f);
		float3 diffuse = Lights[j].DiffuseAndRange.rgb * NdotL * Lights[j].DirectionAndIntensity.w;
		diffuse *= attenuation;
		
		finalLight += diffuse;
		
		// スペキュラー反射(Blinn-Phongモデル)
		float3 halfVec = normalize(lightDir + eyev);
		float specularIntensity = pow(max(dot(normal.xyz, halfVec), 0.0f), Material.Shininess);
		finalSpecular += specularIntensity * Lights[j].DirectionAndIntensity.w * Material.Specular.rgb * attenuation;
	}
	
	//スポットライトのループ
	uint spotStartIndex = DirectionalLightCount + PointLightCount;
	for (uint k = spotStartIndex; k < spotStartIndex + SpotLightCount; k++)
	{
		// ライトベクトルと距離の計算
		float3 lightVec = Lights[k].PositionAndType.xyz - In.WorldPosition.xyz;
		float distance = length(lightVec);
		float3 lightDir = lightVec / distance;
		
		// スポットライトの照射方向（ライトから見た方向）
		float3 spotDirection = normalize(Lights[k].DirectionAndIntensity.xyz);
		
		// ライトの方向とピクセルへの方向の内積
		float cosAngle = dot(-lightDir, spotDirection);
		
		// スポットライトのパラメータ(cpu側でcosineに変換済み)
		// spotParams.x = cos(innerCone/2) (内側コーンの半角のcos値)
		// spotParams.y = cos(outerCone/2) (外側コーンの半角のcos値)
		// spotParams.z = falloff (減衰率)
		float innnerCone = Lights[k].spotParams.x;
		float outerCone = Lights[k].spotParams.y;
		float falloff = Lights[k].spotParams.z;

		// スポットライトの減衰係数
		float spotEffect = 0.0f;
		
		// 範囲チェックと減衰計算
		if (cosAngle <= outerCone)
		{
			// outerConeより外側 - 光が当たらない
			spotEffect = 0.0f;
		}
		else if (cosAngle > innnerCone)
		{
			// innerConeより内側 - 光が最大
			spotEffect = 1.0f;
		}
		else
		{
			// innerConeとouterConeの間 - 減衰計算
			float ratio = (cosAngle - outerCone) / (innnerCone - outerCone);
			spotEffect = pow(ratio, falloff);
		}
		
		// 距離による減衰係数
		float attenuation = 1.0f / (
			Lights[k].attenuation.x + 
			Lights[k].attenuation.y * distance + 
			Lights[k].attenuation.z * distance * distance
		);
		
		float rangeFactor = saturate(1.0f - distance / Lights[k].DiffuseAndRange.w);
		
		// 最終的な減衰
		float finalAttenuation = attenuation * rangeFactor * spotEffect;
		
		// 拡散光
		float NdotL = max(dot(normal.xyz, lightDir), 0.0f);
		
		finalLight += Lights[i].DiffuseAndRange.rgb * NdotL * Lights[i].DirectionAndIntensity.w * finalAttenuation;


		float3 halfVec = normalize(lightDir + eyev);
		float specularIntensity = pow(max(dot(normal.xyz, halfVec), 0.0f), Material.Shininess);
		finalSpecular += specularIntensity * Lights[i].DirectionAndIntensity.w * Material.Specular.rgb * finalAttenuation;
	}

	finalLight = saturate(finalLight);

	outDiffuse.rgb = baseColor.rgb * finalLight;
	outDiffuse.rgb += finalSpecular;

	// シャドウマップの適用
	float shadowAmount = 1.0f;
	for (uint l = 0; l < ShadowLightCount; l++)
	{
		float shadow = CalculateHardShadowWithNormalBias(In.WorldPosition.xyz, normal.xyz, l, 0.005);
		shadowAmount *= shadow;
	}
	
	outDiffuse.rgb *= shadowAmount;

	outDiffuse.a = 1.0f;
}
