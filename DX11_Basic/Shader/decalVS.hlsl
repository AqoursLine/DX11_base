#include "common.hlsl"

struct DecalVertex
{
	float3 position : POSITION0;
};

struct DecalInstance
{
	float4x4 WorldMatrix : WORLD;
	float4 Params : PARAMS; // x: 深度, y: 半径, z: フェード, w: テクスチャインデックス
};

struct DecalVS_INPUT
{
	float3 Position : POSITION0;
	float4x4 InstanceWorld : WORLD;
	float4 InstanceParams : PARAMS;
	uint InstanceID : SV_InstanceID;
};

struct DecalPS_INPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : POSITION0;
	float2 TexCoord : TEXCOORD0;
	float4 Params : PARAMS;
};

void main (in DecalVS_INPUT In, out DecalPS_INPUT Out)
{
	//インスタンスのワールド行列を適用
	float4 worldPos = mul(float4(In.Position, 1.0f), In.InstanceWorld);
	Out.WorldPosition = mul(worldPos, WorldMatrix);
	
	//ビュープロジェクション変換
	float4 viewPos = mul(Out.WorldPosition, ViewMatrix);
	Out.Position = mul(viewPos, ProjectionMatrix);
	
	//テクスチャ座標を計算
	Out.TexCoord = In.Position.xy + 0.5f;
	
	//パラメータを設定
	Out.Params = In.InstanceParams;
}
