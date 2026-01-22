#include "common.hlsl"

//波紋データ
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

	//基本波パラメータ
	float BaseWaveFreq1;
	float BaseWaveFreq2;
	float BaseWaveFreq3;
	float BaseWaveSpeed1;
	float BaseWaveSpeed2;
	float BaseWaveSpeed3;
	float WaveSharpness; // 波頭の鋭さ（べき乗の指数）
	float padding3;
	
	// 環境マッピングパラメータ（ピクセルシェーダーで使用）
	float ReflectionStrength;
	float RefractionStrength;
	float FresnelPower;
	float WaterClarityDepth;

	//波紋データ（先頭から詰まっている）
	RippleData Ripples[32];
}

void main(in VS_INPUT input, out PS_INPUT output)
{
	//ワールド位置を計算
	float4 worldPos = mul(input.Position, WorldMatrix);
		
	//ビュー座標とプロジェクション座標を計算
	float4 viewPos = mul(worldPos, ViewMatrix);

	output.Position = mul(viewPos, ProjectionMatrix);
	output.WorldPosition = worldPos;

	output.Normal = float4(normalize(mul(input.Normal.xyz, (float3x3) WorldMatrix)), 0.0f);
	output.Tangent = float4(normalize(mul(input.Tangent.xyz, (float3x3) WorldMatrix)), 0.0f);
	
	//その他のデータを出力
	output.TexCoord = input.TexCoord.xy;
	output.Diffuse = input.Diffuse;
}
