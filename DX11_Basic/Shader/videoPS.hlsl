#include "common.hlsl"

// テクスチャとサンプラーステートの宣言
Texture2D txYUV : register(t0);
SamplerState samLinear : register(s0);

// yuvからrgbへの変換関数
float3 YUVtoRGB(float3 yuv)
{
	// BT.601の係数を使用
	static const float3 yuvCoef_r = { 1.164f, 0.000f, 1.596f };
	static const float3 yuvCoef_g = { 1.164f, -0.392f, -0.813f };
	static const float3 yuvCoef_b = { 1.164f, 2.017f, 0.000f };
	
	yuv -= float3(0.0625f, 0.5f, 0.5f);
	
	return saturate(float3(
		dot(yuv, yuvCoef_r),
		dot(yuv, yuvCoef_g),
		dot(yuv, yuvCoef_b)
	));
}


// メインのピクセルシェーダー
void main(in PS_INPUT input, out float4 output : SV_TARGET)
{
	float2 tex = input.TexCoord.xy;
	float y = txYUV.Sample(samLinear, float2(tex.x, tex.y * 0.5)).r;
	float u = txYUV.Sample(samLinear, float2(tex.x * 0.5, 0.50 + tex.y * 0.25)).r;
	float v = txYUV.Sample(samLinear, float2(tex.x * 0.5, 0.75 + tex.y * 0.25)).r;
	
	output = float4(YUVtoRGB(float3(y, u, v)), 1.f);
}
