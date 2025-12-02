#include "common.hlsl"

struct ParticleInstanceData
{
	float3 position;
	float size;
	float4 color;
	float2 texOffset;
	float rotation;
	float padding; // パディング
};

StructuredBuffer<ParticleInstanceData> g_instanceDataBuffer : register(t0);

struct VS_PARTICLE_INPUT
{
	float2 Offset : OFFSET; // 頂点オフセット
	float2 TexCoord : TEXCOORD; // テクスチャ座標
};

struct PS_PARTICLE_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float4 Color : COLOR;
};

void main(in VS_PARTICLE_INPUT input, out PS_PARTICLE_INPUT output, uint instanceID : SV_InstanceID)
{
	ParticleInstanceData instance = g_instanceDataBuffer[instanceID];
	
	// 回転+スケーリング行列の計算
	float cosR = cos(instance.rotation);
	float sinR = sin(instance.rotation);
	float size = instance.size;
	
	float3x3 localTransform = float3x3(
		cosR * size, -sinR * size, 0.0,
		sinR * size,  cosR * size, 0.0,
		0.0,          0.0,         1.0
	);

	// ローカル変換を適用
	float3 localPos = mul(float3(input.Offset, 0.0f), localTransform);
	
	// ビルボード変換
	float3 billboardPos = mul(float4(localPos, 0.0f), BillboardMatrix).xyz;

	// ワールド位置の計算
	float3 worldPos = instance.position + billboardPos;
	
	// クリップ空間への変換
	float4 viewPos = mul(float4(worldPos, 1.0f), ViewMatrix);
	output.Position = mul(viewPos, ProjectionMatrix);

	output.TexCoord = input.TexCoord + instance.texOffset;
	output.Color = instance.color;
}
