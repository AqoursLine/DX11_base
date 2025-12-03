#include "common.hlsl"

// GPU側パーティクル構造体
struct Particle
{
	float3 position;
	float3 velocity;
	float4 color;
	float size;
	float life;
	float maxLife;
	float rotation;
	float rotationSpeed;
	uint active;
};

// StructuredBuffer
StructuredBuffer<Particle> g_particles : register(t0);

// 入力
struct VS_PARTICLE_INPUT
{
	float2 offset : POSITION; // 頂点オフセット
	float2 texCoord : TEXCOORD; // テクスチャ座標
	uint instanceID : SV_InstanceID; // インスタンスID
};

// 出力
struct PS_PARTICLE_INPUT
{
	float4 position : SV_POSITION; // クリップ空間位置
	float2 texCoord : TEXCOORD0; // テクスチャ座標
	float4 color : COLOR0; // 頂点カラー
	float cullDistance : SV_CullDistance0; // カリング距離
};

void main(in VS_PARTICLE_INPUT input, out PS_PARTICLE_INPUT output)
{
	Particle p = g_particles[input.instanceID];
	
	// 非アクティブなパーティクルは描画しない
	if (p.active == 0)
	{
		output.cullDistance = -1.0f; // カリング距離を負に設定
		output.position = float4(0.0f, 0.0f, 0.0f, 1.0f);
		output.texCoord = input.texCoord;
		output.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
		return;
	}

	output.cullDistance = 1.0f; // カリング距離を正に設定
	
	// 回転＋スケーリング行列
	float cosR = cos(p.rotation);
	float sinR = sin(p.rotation);
	float size = p.size;
	
	float3x3 localTransform = float3x3(
		cosR * size, -sinR * size, 0.0f,
		sinR * size,  cosR * size, 0.0f,
		0.0f,         0.0f,        1.0f
	);

	// ビルボード変換
	float3 localPos = mul(float3(input.offset, 0.0f), localTransform);
	float3 billboardPos = mul(float4(localPos, 0.0f), BillboardMatrix).xyz;
	float3 worldPos = p.position + billboardPos;
	
	// クリップ空間変換
	float4 viewPos = mul(float4(worldPos, 1.0f), ViewMatrix);
	output.position = mul(viewPos, ProjectionMatrix);
	
	output.texCoord = input.texCoord;
	output.color = p.color;
}
