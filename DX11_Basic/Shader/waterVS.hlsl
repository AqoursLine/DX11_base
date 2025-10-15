#include "common.hlsl"

//波紋データ
struct RippleData
{
	float4 PositionAndTime;	// xyz:位置 w:開始時間
	float4 Params;			// x:振幅 y:波長 z:速度 w:使用フラグ
};

cbuffer WaterConstantBuffer : register(b7)
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
	output.TexCoord = input.TexCoord.xy;
	output.Normal = float4(worldNormal, 0.0f);
	output.Diffuse = input.Diffuse;
}

