#include "common.hlsl"

//波紋データ
struct RippleData
{
	float4 PositionAndTime;	// xyz:位置 w:開始時間
	float4 Params;			// x:振幅 y:波長 z:速度 w:使用フラグ
};

//航跡波データ
struct WakeTrailData
{
	float4 StartPos;	// xyz:開始位置 w:時間
	float4 EndPos;		// xyz:終了位置 w:強さ
	float4 Params;		// x:幅, y:長さ, z:寿命, w:使用フラグ
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
	float BaseWaveSpeed1;
	float BaseWaveSpeed2;
	float BaseWaveSpeed3;
	float padding2[2];

	//波紋データ
	RippleData Ripples[10];
	WakeTrailData WakeTrails[20];
}

//航跡波の高さを計算する関数
float CalculateWakeHeight(float3 worldPos, float time)
{
	float wakeHeight = 0.0f;
	
	[unroll]
	for (int i = 0; i < 20; i++)
	{
		if (WakeTrails[i].Params.w <= 0.0f) continue; // 使用フラグが立っていない場合はスキップ
		
		float3 startPos = WakeTrails[i].StartPos.xyz;
		float3 endPos = WakeTrails[i].EndPos.xyz;
		float wakeTime = WakeTrails[i].StartPos.w;
		float intensity = WakeTrails[i].EndPos.w;
		float width = WakeTrails[i].Params.x;
		float length = WakeTrails[i].Params.y;
		float lifeTime = WakeTrails[i].Params.z;
		
		//航跡線分に対する最接近点を計算
		float3 wakeVec = endPos - startPos;
		float3 pointVec = worldPos - startPos;
		
		float wakeLength = sqrt(dot(wakeVec, wakeVec));
		if (wakeLength < 0.1f) continue; // 長さがほぼ0の場合はスキップ
		
		float3 wakeDir = wakeVec / wakeLength;
		float projLength = dot(pointVec, wakeDir);
		
		//航跡の範囲外ならスキップ
		if (projLength < 0.0f || projLength > wakeLength) continue;
		
		//最接近点を計算
		float3 closestPoint = startPos + wakeDir * projLength;
		
		//最接近点からの距離
		float3 offsetVec = worldPos - closestPoint;
		float lateralDistance = sqrt(dot(offsetVec, offsetVec));
		
		//航跡の幅内かチェック
		if (lateralDistance > width) continue;
		
		//V字型の航跡パターンを作成
		float normalizedPos = projLength / wakeLength; // 0から1の範囲
		float normalizedLateral = lateralDistance / width; // 0から1の範囲
		
		//時間減衰
		float timeAttenuation = exp(-wakeTime * 0.1f);
		
		//距離減衰
		float lateralAttenuation = cos(normalizedLateral * 3.14159f * 0.5f);
		
		//長さ方向の減衰
		float lengthAttenuation = 1.0f - normalizedPos * 0.3f; // 徐々に減衰
		
		//V字パターン
		float kelvinAngle = 19.47f * (3.14159f / 180.0f); // ケルビン角度
		float expectedLateral = normalizedPos * wakeLength * tan(kelvinAngle);
		
		float kelvinFactor = 1.0f;
		if (lateralDistance > expectedLateral * 0.5f)
		{
			kelvinFactor = exp(-(lateralDistance - expectedLateral * 0.5f) / width);
		}
		
		//波の高さを計算
		float wavePhase = (projLength * 0.5f + lateralDistance * 2.0f - time * 3.0f);
		float amplitude = intensity * timeAttenuation * lateralAttenuation * lengthAttenuation * kelvinFactor;
		
		wakeHeight += sin(wavePhase) * amplitude * 0.3f;
		
		//追加の細かい波
		float smallWavePhase = (projLength * 2.0f + lateralDistance * 5.0f - time * 8.0f);
		wakeHeight += sin(smallWavePhase) * amplitude * 0.15f;
		
	}
	
	return wakeHeight;
}

//波の高さを計算する関数
float CalculateWaveHeight(float3 worldPos, float time)
{
	float height = 0.0f;
	float x = worldPos.x;
	float z = worldPos.z;
	
	//基本的な波
	height += sin(x * BaseWaveFreq1 + time * BaseWaveSpeed1) * WaveHeight * 0.3f;
	height += sin(z * BaseWaveFreq2 + time * BaseWaveSpeed2) * WaveHeight * 0.2f;
	height += sin((x + z) * BaseWaveFreq3 + time * BaseWaveSpeed3) * WaveHeight * 0.5f;
	
	//波紋効果
	[unroll]
	for (int i = 0; i < 10; i++)
	{
		if (Ripples[i].Params.w > 0.0f)
		{
			float3 ripplePos = Ripples[i].PositionAndTime.xyz;
			float rippleTime = Ripples[i].PositionAndTime.w;
			float amplitude = Ripples[i].Params.x;
			float frequency = Ripples[i].Params.y;
			float speed = Ripples[i].Params.z;
			
			float dx = x - ripplePos.x;
			float dz = z - ripplePos.z;
			float distance = sqrt(dx * dx + dz * dz);
			
			if (distance < speed * rippleTime && rippleTime > 0.0f)
			{
				float wavePahse = frequency * (distance - speed * rippleTime);
				float attenuation = exp(-rippleTime * 0.5f);
				float distanceAttenuation = 1.0f / (1.0f + distance * 0.01f);
				height += sin(wavePahse) * amplitude * attenuation * distanceAttenuation;

			}
		}

	}
	
	height += CalculateWakeHeight(worldPos, time);
	
	return height;

}

//法線を計算する関数
float3 CalculateNormal(float3 worldPos, float time)
{
	float delta = 0.1f;
	
	float heightL = CalculateWaveHeight(worldPos + float3(-delta, 0, 0), time);
	float heightR = CalculateWaveHeight(worldPos + float3(delta, 0, 0), time);
	float heightD = CalculateWaveHeight(worldPos + float3(0, 0, -delta), time);
	float heightU = CalculateWaveHeight(worldPos + float3(0, 0, delta), time);
	
	float3 normal;
	normal.x = (heightL - heightR) / (2.0f * delta);
	normal.y = 1.0f;
	normal.z = (heightD - heightU) / (2.0f * delta);
	
	return normalize(normal);
}

void main(in VS_INPUT input, out PS_INPUT output)
{
	//ワールド位置を計算
	float4 worldPos = mul(input.Position, WorldMatrix);
	
	//波の高さを計算
	float waveHeight = CalculateWaveHeight(worldPos.xyz, Time);
	worldPos.y += waveHeight;
	
	//法線を計算
	float3 worldNormal = CalculateNormal(worldPos.xyz, Time);
	worldNormal = normalize(mul(worldNormal, (float3x3) WorldMatrix));
	
	//ビュー座標とプロジェクション座標を計算
	float4 viewPos = mul(worldPos, ViewMatrix);
	output.Position = mul(viewPos, ProjectionMatrix);
	
	//その他のデータを出力
	output.WorldPosition = worldPos;
	output.TexCoord = input.TexCoord;
	output.Normal = float4(worldNormal, 0.0f);
	output.Diffuse = input.Diffuse;
}

