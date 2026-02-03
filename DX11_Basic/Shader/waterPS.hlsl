#include "common.hlsl"

struct RippleData
{
	float4 PositionAndTime; // xyz:位置 w:開始時間
	float4 Params; // x:振幅 y:波長 z:速度 w:使用フラグ（常に1.0）
};

cbuffer WaterConstantBuffer : register(b7)
{
	float Time;
	float WaveHeight;
	float WaterSize;
	int ActiveRippleCount; // アクティブな波紋の数（動的ループ用）
    
    // 基本波パラメータ
	float BaseWaveFreq1;
	float BaseWaveFreq2;
	float BaseWaveFreq3;
	float BaseWaveSpeed1;
	float BaseWaveSpeed2;
	float BaseWaveSpeed3;
	float WaveSharpness; // 波頭の鋭さ（べき乗の指数）
	float padding3;
    
    // 環境マッピングパラメータ
	float ReflectionStrength; // 反射強度 (0.0-1.0)
	float RefractionStrength; // 屈折強度 (0.0-1.0)
	float FresnelPower; // フレネル効果の強さ
	float WaterClarityDepth; // 水の透明度（深さ）

    // 波紋データ（先頭から詰まっている）
	RippleData Ripples[64];
}

// テクスチャとサンプラー
TextureCube EnvironmentMap : register(t0); // 環境マップ（キューブマップ）
Texture2D NormalMapTexture : register(t1); // 法線マップ（オプション）
Texture2D FoamTexture : register(t2); // 泡テクスチャ
SamplerState LinearSampler : register(s0);

void main(in PS_INPUT input, out float4 outDiffuse : SV_TARGET)
{
    // 水の基本色
	float3 deepWaterColor = float3(0.1f, 0.4f, 0.6f); // 深い水の色（より濃く）
	float3 shallowWaterColor = float3(0.2f, 0.6f, 0.9f); // 浅い水の色
    
    // 法線を正規化
	float3 normal = normalize(input.Normal.xyz);
    
    // 法線マップを2層で適用（異なる速度とスケールでスクロール）
    // レイヤー1: 大きな波
	float2 normalUV1 = input.TexCoord * 8.0f + float2(Time * 0.03f, Time * 0.02f);
	float3 normalMap1 = NormalMapTexture.Sample(LinearSampler, normalUV1).xyz * 2.0f - 1.0f;
    
    // レイヤー2: 小さな波（逆方向にスクロール）
	float2 normalUV2 = input.TexCoord * 15.0f + float2(-Time * 0.05f, Time * 0.04f);
	float3 normalMap2 = NormalMapTexture.Sample(LinearSampler, normalUV2).xyz * 2.0f - 1.0f;
    
    // 2つの法線マップを合成
	float3 detailNormal = normalize(normalMap1 + normalMap2 * 0.5f);
	detailNormal.xy *= 0.4f; // 強度調整
    
    // 頂点シェーダーからの法線と合成
	normal = normalize(normal + detailNormal);
    
    // ライティング計算
	float3 lightDir = normalize(Lights[0].DirectionAndIntensity.xyz);
	float3 lightColor = Lights[0].DiffuseAndRange.rgb;
    
    // ランバート拡散反射
	float NdotL = max(0.0f, dot(-lightDir, normal));
	float3 diffuse = lightColor * NdotL;
    
    // ビュー方向
	float3 viewDirection = normalize(CameraPosition.xyz - input.WorldPosition.xyz);
    
    // フレネル効果（改良版）
	float fresnel = pow(1.0f - max(0.0f, dot(viewDirection, normal)), FresnelPower);
	fresnel = saturate(fresnel);
    
    // === 環境マッピング ===
    
    // 反射ベクトルを計算
	float3 reflectionVector = reflect(-viewDirection, normal);
	float3 reflectionColor = EnvironmentMap.Sample(LinearSampler, reflectionVector).rgb;
    
    // 屈折ベクトルを計算（水の屈折率 約1.33）
	float refractionRatio = 1.0f / 1.33f;
	float3 refractionVector = refract(-viewDirection, normal, refractionRatio);
	float3 refractionColor = EnvironmentMap.Sample(LinearSampler, refractionVector).rgb;
    
    // フレネル効果で反射と屈折をブレンド
	float3 envColor = lerp(refractionColor, reflectionColor, fresnel);
	envColor *= ReflectionStrength;
    
    // === スペキュラー反射（太陽光のきらめき）===
	float3 halfVector = normalize(-lightDir + viewDirection);
	float NdotH = max(0.0f, dot(normal, halfVector));
	float3 specular = lightColor * pow(NdotH, 128.0f) * 1.2f;
    
    // 水深による色の変化
	float depth = abs(input.WorldPosition.y);
	float depthFactor = saturate(depth / WaterClarityDepth);
	float3 waterColor = lerp(shallowWaterColor, deepWaterColor, depthFactor);
    
    // === 波の頂点を白くする処理（改良版）===
	float baseWaterLevel = 0.0f;
	float waveHeight = input.WorldPosition.y - baseWaterLevel;
    
    // 波の高さに基づく白色効果
	float whiteFactor = 0.0f;
	if (waveHeight > 0.2f)
	{
		whiteFactor = pow(saturate((waveHeight - 0.2f) / 2.5f), 0.5f);
	}
    
    // 法線の傾きから白波を検出
	float slopeFactor = 1.0f - abs(dot(normal, float3(0.0f, 1.0f, 0.0f)));
	slopeFactor = pow(saturate(slopeFactor), 1.5f);
    
    // 波の頂点検出（白波生成）
	float peakFactor = 0.0f;
	if (waveHeight > 0.5f && slopeFactor > 0.3f)
	{
		peakFactor = saturate((waveHeight - 0.5f) * slopeFactor * 2.5f);
	}
    
    // 泡テクスチャの適用
	float2 foamUV = input.TexCoord * 5.0f + float2(Time * 0.1f, Time * 0.08f);
	float foamMask = FoamTexture.Sample(LinearSampler, foamUV).r;
	float foamIntensity = peakFactor * foamMask;
    
    // 最終的な白色強度
	float totalWhiteness = max(whiteFactor, foamIntensity);
	totalWhiteness = saturate(totalWhiteness);
    
    // === 波紋による色の変化 ===
	float rippleIntensity = 0.0f;
    // 動的ループ（[unroll]削除）、条件分岐不要
	for (int i = 0; i < ActiveRippleCount; i++)
	{
        // 先頭から詰まっているので、全てアクティブ
		float3 ripplePos = Ripples[i].PositionAndTime.xyz;
		float rippleStartTime = Ripples[i].PositionAndTime.w;
		float amplitude = Ripples[i].Params.x;
        
		float dx = input.WorldPosition.x - ripplePos.x;
		float dz = input.WorldPosition.z - ripplePos.z;
		float distance = sqrt(dx * dx + dz * dz);
        
		if (distance < 50.0f)
		{
			float attenuation = exp(-rippleStartTime * 0.3f);
			float distanceAttenuation = 1.0f - saturate(distance / 50.0f);
			rippleIntensity += amplitude * attenuation * distanceAttenuation * 0.2f;
		}
	}
    
    // 時間による水面の細かい動き（カスティクス風）
	float2 animUV = input.TexCoord + float2(sin(Time * 0.1f), cos(Time * 0.15f)) * 0.01f;
	float caustics = sin(animUV.x * 30.0f + Time * 2.0f) * sin(animUV.y * 25.0f + Time * 1.8f);
	caustics = max(0.0f, caustics) * 0.15f * (1.0f - depthFactor); // 浅いところだけ
    
    // アンビエント光
	float3 ambient = Material.Ambient.rgb * waterColor * 0.3f;
    
    // === 最終色を計算 ===
    // 基本色 + ライティング + 環境マップ + スペキュラー + カスティクス
	float3 finalColor = ambient + waterColor * diffuse * 0.5f + envColor + specular * fresnel + caustics;
    
    // 波紋効果を追加
	finalColor += rippleIntensity * float3(0.5f, 0.7f, 0.9f);
    
    // 白波を追加
	float3 whiteColor = float3(1.0f, 1.0f, 1.0f);
	finalColor = lerp(finalColor, whiteColor, totalWhiteness);
    
    // 非常に強い白波にはさらに輝度を追加
	if (totalWhiteness > 0.7f)
	{
		finalColor += float3(0.4f, 0.4f, 0.4f) * (totalWhiteness - 0.7f);
	}
    
    // 入力カラーを考慮
	finalColor *= input.Diffuse.rgb;
	
	// デバッグ用
//	finalColor = envColor;
    
    // 透明度設定（フレネル効果で調整）
	float alpha = input.Diffuse.a * (0.6f + fresnel * 0.4f);
	alpha = saturate(alpha);
    
	outDiffuse = float4(finalColor, alpha);
}
