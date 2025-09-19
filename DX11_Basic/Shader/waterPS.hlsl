#include "common.hlsl"

struct RippleData
{
	float4 PositionAndTime; // xyz:位置 w:開始時間
	float4 Params; // x:振幅 y:波長 z:速度 w:使用フラグ
};
struct WakeTrailData
{
	float4 StartPos; // xyz:開始位置 w:時間
	float4 EndPos; // xyz:終了位置 w:強さ
	float4 Params; // x:幅, y:長さ, z:寿命, w:使用フラグ
};


cbuffer WaterConstantBuffer : register(b6)
{
	float Time;
	float WaveHeight;
	float WaterSize;
	float padding1;
	
	//基本波パラメータ
	float BaseWaveFreq1;
	float BaseWaveFreq2;
	float BaseWaveFreq3;
	float BeseWaveSpeed1;
	float BaseWaveSpeed2;
	float BaseWaveSpeed3;
	float padding2[2];

	//波紋データ
	RippleData Ripples[10];
	WakeTrailData WakeTrails[20];
}

void main(in PS_INPUT input, out float4 outDiffuse : SV_TARGET)
{
	//水の基本色
	float3 deepWaterColor = float3(0.1f, 0.3f, 0.5f); //深い水の色
	float3 shallowWaterColor = float3(0.3f, 0.7f, 0.9f); //浅い水の色
	float3 wakeColor = float3(0.7f, 0.9f, 1.9f); //航跡波の色
	
	//法線を正規化
	float3 normal = normalize(input.Normal.xyz);
	
	//ライティング計算
	float3 lightDir = normalize(Light.Direction.xyz);
	float3 lightColor = Light.Diffuse.rgb;
	
	//ランバート拡散反射
	float NdotL = max(0.0f, dot(-lightDir, normal));
	float3 diffuse = lightColor * NdotL;
	
	//スペキュラー反射(ブリン・フォン反射モデル)
	float3 viewDirection = normalize(CameraPosition.xyz - input.WorldPosition.xyz);
	float3 halfVector = normalize(-lightDir + viewDirection);
	float NdotH = max(0.0f, dot(normal, halfVector));
	float3 specular = lightColor * pow(NdotH, 64.0f) * 0.8f;
	
	//フレネル効果(簡易版)
	float fresnel = pow(1.0f - max(0.0f, dot(viewDirection, normal)), 2.0f);
	fresnel = lerp(0.1f, 1.0f, fresnel);
	
	//水深による色の変化
	float depth = abs(input.WorldPosition.y);
	float depthFactor = saturate(depth / 2.0f);
	float3 waterColor = lerp(shallowWaterColor, deepWaterColor, depthFactor);
	
	//波の頂点を真っ白にする処理
	float baseWaterLevel = 0.0f; //基準となる水面の高さ
	float waveHeight = input.WorldPosition.y - baseWaterLevel;
	
	//波の高さに基づく白色効果
	float waveHeightNormalized = saturate(waveHeight / 3.0f);
	float whiteFactor = 0.0f;
	
	//波が高いほど白くする(非線形カーブ)
	if (waveHeight > 0.2f)
	{
		//平方根カーブ
		whiteFactor = pow(saturate((waveHeight - 0.2f) / 2.0f), 0.5f);
	}
	
	//法線の傾きからも白色を追加
	float slopeFactor = 1.0f - abs(dot(normal, float3(0.0f, 1.0f, 0.0f)));
	slopeFactor = pow(saturate(slopeFactor), 2.0f);

	//波の頂点検出
	float peakFactor = 0.0f;
	if (waveHeight > 0.5f && slopeFactor > 0.3f)
	{
		peakFactor = saturate((waveHeight - 0.5f) * slopeFactor * 2.0f);
	}
	
	//最終的な白色強度を計算
	float totalWhiteness = max(whiteFactor, peakFactor);
	totalWhiteness = saturate(totalWhiteness);
	
	//波紋による色の変化
	float rippleIntensity = 0.0f;
	[unroll]
	for (int i = 0; i < 10; i++)
	{
		if (Ripples[i].Params.w <= 0.0f) continue; // 使用フラグが立っていない場合はスキップ
		
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
	
	//航跡波による色の変化
	float wakeIntensity = 0.0f;
	[unroll]
	for (i = 0; i < 20; i++)
	{
		if (WakeTrails[i].Params.w <= 0.0f)	continue; // 使用フラグが立っていない場合はスキップ
		
		float3 startPos = WakeTrails[i].StartPos.xyz;
		float3 endPos = WakeTrails[i].EndPos.xyz;
		float wakeTime = WakeTrails[i].StartPos.w;
		float intensity = WakeTrails[i].EndPos.w;
		float width = WakeTrails[i].Params.x;
		
		//航跡線分に対する最近接点を計算
		float3 wakeVec = endPos - startPos;
		float3 pointVec = input.WorldPosition.xyz - startPos;
		
		float wakeLength = length(wakeVec);
		if (wakeLength < 0.1f) continue; // 長さがほぼ0の場合はスキップ)
		
		float3 wakeDir = wakeVec / wakeLength;
		float projLength = dot(pointVec, wakeDir);
		
		//航跡の範囲外ならスキップ
		if (projLength < 0.0f || projLength > wakeLength) continue;
		
		//最近接点を計算
		float3 closestPoint = startPos + wakeDir * projLength;
		
		//最近接点からの距離
		float lateralDistance = length(input.WorldPosition.xyz - closestPoint);
		
		//航跡の影響範囲内かチェック
		if (lateralDistance < width * 1.5f)
		{
			float timeAttenuation = exp(-wakeTime * 0.08f);
			float distanceAttenuation = 1.0f - saturate(lateralDistance / (width * 1.5f));
			float wakeEffect = intensity * timeAttenuation * distanceAttenuation;
			
			//航跡中心は白っぽく
			if (lateralDistance < width * 0.3f)
			{
				wakeIntensity += wakeEffect * 0.6f;
			}
			else
			{
				wakeIntensity += wakeEffect * 0.3f;
			}
		}
	}
	
	//時間による水面の動き
	float2 animUV = input.TexCoord + float2(sin(Time * 0.1f), cos(Time * 0.15f)) * 0.01f;
	float foam = sin(animUV.x * 20.0f + Time * 2.0f) * sin(animUV.y * 15.0f + Time * 1.5f);
	foam = max(0.0f, foam) * 0.1f;
	
	//アンビエント光を加算
	float3 ambient = Light.Ambient.rgb * waterColor;
	
	//最終色を計算
	float3 finalColor = ambient + waterColor * diffuse + specular * fresnel + foam;

	//波紋効果を追加
	finalColor += rippleIntensity * float3(0.6f, 0.8f, 1.0f);
	
	//航跡波効果を追加
	finalColor = lerp(finalColor, wakeColor, wakeIntensity);
	finalColor += wakeIntensity * float3(0.8f, 0.8f, 0.8f);

	//波の頂点を白く
	float3 whiteColor = float3(1.0f, 1.0f, 1.0f);
	finalColor = lerp(finalColor, whiteColor, totalWhiteness);

	//さらに強い白色効果
	if (totalWhiteness > 0.7f)
	{
		finalColor += float3(0.3f, 0.3f, 0.3f);

	}
	//入力カラーを考慮
	finalColor *= input.Diffuse.rgb;
	
	//透明度設定(航跡部分は少し不透明に)
	float alpha = input.Diffuse.a * (0.7f + fresnel * 0.3f + wakeIntensity * 0.2f);
	alpha = saturate(alpha);
	
	outDiffuse = float4(finalColor, alpha);

}
