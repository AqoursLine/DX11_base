#include "common.hlsl"

// インスタンスデータ
struct VS_INSTANCE_INPUT
{
	float4 Position : POSITION; // ワールド座標
	float4 Normal : NORMAL; // 法線
	float4 TexCoord : TEXCOORD0; // テクスチャ座標
	float4 Tangent : TANGENT0; // 接線
	float4 Diffuse : COLOR0; // 頂点カラー
	
	// インスタンスごとのデータ
	float4 InstancePosAndSize : TEXCOORD1; // xyz:位置, w:サイズ
	float4 InstanceColor : COLOR1; // パーティクルカラー
	float4 InstanceRotation : TEXCOORD2; // xyz:回転角度, w:未使用
};

// ピクセルシェーダーへの出力
struct VS_PARTICLE_OUTPUT
{
	float4 Position : SV_POSITION; // クリップ空間位置
	float2 TexCoord : TEXCOORD0; // テクスチャ座標
	float4 Color : COLOR0; // 頂点カラー
};

// 頂点シェーダー
void main(in VS_INSTANCE_INPUT inputm, out VS_PARTICLE_OUTPUT output)
{
	// インスタンスデータを取得
	float3 particlePos = inputm.InstancePosAndSize.xyz;
	float particleSize = inputm.InstancePosAndSize.w;
	float rotation = inputm.InstanceRotation.x;
	
	// ビルボード用の座標系を計算
	float3 cameraRight = float3(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
	float3 cameraUp = float3(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
	
	// 回転行列を適用
	float cosRot = cos(rotation);
	float sinRot = sin(rotation);
	
	float2 rotatedPos;
	rotatedPos.x = inputm.Position.x * cosRot - inputm.Position.y * sinRot;
	rotatedPos.y = inputm.Position.x * sinRot + inputm.Position.y * cosRot;
	
	// ビルボードの頂点位置を計算
	float3 vertexPosition = particlePos;
	vertexPosition += cameraRight * rotatedPos.x * particleSize;
	vertexPosition += cameraUp * rotatedPos.y * particleSize;
	
	// ワールド変換
	float4 worldPos = float4(vertexPosition, 1.0f);
	float4 viewPos = mul(worldPos, ViewMatrix);
	output.Position = mul(viewPos, ProjectionMatrix);
	
	// テクスチャ座標とカラーを設定
	output.TexCoord = inputm.TexCoord.xy;
	output.Color = inputm.InstanceColor;

}

