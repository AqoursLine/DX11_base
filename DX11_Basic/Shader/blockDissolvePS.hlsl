#include "common.hlsl"

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

float rand(float2 seed)
{
	return frac(sin(dot(seed, float2(127.1, 311.7))) * 43758.5453);
}

void main(in PS_INPUT input, out float4 output : SV_TARGET)
{
	float blockCountX = params1.x;	// 横の分割数
	float blockCountY = params1.y;	// 縦の分割数
	float progress = params1.z;		// 進行度
	float scatter = params1.w;		// 散乱度
	
	// ブロックインデックスを計算
	float2 blockIndex = floor(input.TexCoord * float2(blockCountX, blockCountY));
	
	// ブロックごとの乱数を生成
	float rnd = rand(blockIndex);
	// ブロックの閾値を計算
	float threshold = (blockIndex.x / blockCountX) + rnd * scatter;

	threshold = threshold / (1.0 + scatter);

	clip(progress - threshold);
	
	// テクスチャから色をサンプリング
	output = g_texture.Sample(g_sampler, input.TexCoord);
	
	// アルファ破棄
	clip(output.a - 0.01);
	
	output *= input.Diffuse;
}

