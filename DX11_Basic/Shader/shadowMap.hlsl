Texture2DArray shadowMapArray : register(t10);
SamplerComparisonState shadowSampler : register(s10);

#define MAX_SHDOW_LIGHTS 8

cbuffer ShadowLightBuffer : register(b10)
{
	uint ShadowLightCount;
	float3 padding; // パディング
	matrix LightViewProjBiasMatrices[MAX_SHDOW_LIGHTS];
}

// ハードシャドウ
float CalculateHardShadow(float3 worldPos, uint lightIndex)
{
	// ライトインデックスが範囲外なら影なし
	if (lightIndex >= ShadowLightCount)
	{
		return 1.0f;
	}
	
	// ワールド座標をライトのビュー射影バイアスマトリックスで変換
	float4 shadowCoord = mul(float4(worldPos, 1.0f), LightViewProjBiasMatrices[lightIndex]);
	
	// 透視除算
	shadowCoord /= shadowCoord.w;
	
	// ハードシャドウのサンプリング
	return shadowMapArray.SampleCmpLevelZero(
		shadowSampler,
		float3(shadowCoord.xy, lightIndex),
		shadowCoord.z
	);
}

float CalculateHardShadowWithNormalBias(float3 worldPos, float3 normal, uint lightIndex, float normalBias)
{
	// ライトインデックスが範囲外なら影なし
	if (lightIndex >= ShadowLightCount)
	{
		return 1.0f;
	}
	
	// 法線バイアスを適用
	float3 biasedWorldPos = worldPos + normalize(normal) * normalBias;
	
	// ワールド座標をライトのビュー射影バイアスマトリックスで変換
	float4 shadowCoord = mul(float4(biasedWorldPos, 1.0f), LightViewProjBiasMatrices[lightIndex]);
	
	// 透視除算
	shadowCoord.xyz /= shadowCoord.w;
		
	// ハードシャドウのサンプリング
	return shadowMapArray.SampleCmpLevelZero(
		shadowSampler,
		float3(shadowCoord.xy, lightIndex),
		shadowCoord.z
	);
}
