#include "common.hlsl"

// GPU側パーティクル構造体
struct Particle
{
	float3 position;
	float3 velocity;
	float4 color;
	float3 upVector;
	float size;
	float life;
	float maxLife;
	float rotation;
	float rotationSpeed;
	uint active;
	float particlePadding;
};

StructuredBuffer<Particle> g_particles : register(t0);
StructuredBuffer<uint> g_activeIndices : register(t1);

struct VS_PARTICLE_INPUT
{
	float2 offset : POSITION; // 頂点オフセット
	float2 texCoord : TEXCOORD; // テクスチャ座標
	uint instanceID : SV_InstanceID; // インスタンスID
};

struct PS_PARTICLE_INPUT
{
	float4 position : SV_POSITION; // クリップ空間位置
	float2 texCoord : TEXCOORD0; // テクスチャ座標
	float4 color : COLOR0; // 頂点カラー
};

void main(in VS_PARTICLE_INPUT input, out PS_PARTICLE_INPUT output)
{
	// アクティブなパーティクルのインデックスを取得
	uint particleIndex = g_activeIndices[input.instanceID];
	Particle p = g_particles[particleIndex];
	
	// 回転＋スケーリング行列
	float cosR = cos(p.rotation);
	float sinR = sin(p.rotation);
	float size = p.size;
	
	float3x3 localTransform = float3x3(
		cosR * size, -sinR * size, 0.0f,
		sinR * size,  cosR * size, 0.0f,
		0.0f,         0.0f,        1.0f
	);
	
	// 頂点オフセットを変換
	float3 localPos = mul(float3(input.offset, 0.0f), localTransform);
	float3 billboardPos = mul(float4(localPos, 0.0f), BillboardMatrix).xyz;
	float3 worldPos = p.position + billboardPos;
	
	// クリップ空間変換
	float4 viewPos = mul(float4(worldPos, 1.0f), ViewMatrix);
	output.position = mul(viewPos, ProjectionMatrix);

	output.texCoord = input.texCoord;
	output.color = p.color;
}
