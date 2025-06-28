#include "common.hlsl"

Texture2D<float> g_heightMap : register(t0);
SamplerState g_heightMapSampler : register(s0);

struct TerrainVertex
{
	float3 Position : POSITION0;
	float3 Normal : NORMAL0;
	float3 Texcoord : TEXCOORD0;
	float3 Tangent : TANGENT0;
};

void main (inout TerrainVertex In, out PS_INPUT Out)
{
	//ハイトマップから高さを取得
	float height = g_heightMap.SampleLevel(g_heightMapSampler, In.Texcoord.xy, 0);

	//頂点位置にハイトマップの高さを適用
	float4 worldPosition = float4(In.Position.x, In.Position.y + height * 10.0f, In.Position.z, 1.0f);
	
	//ワールド座標変換
	Out.WorldPosition = mul(worldPosition, WorldMatrix);
	
	//ビュープロジェクション変換
	float4 viewPosition = mul(Out.WorldPosition, ViewMatrix);
	Out.Position = mul(viewPosition, ProjectionMatrix);
	
	//法線変換
	Out.Normal = normalize(mul(float4(In.Normal, 0.0f), WorldMatrix));
	
	//テクスチャ座標変換
	Out.TexCoord = In.Texcoord.xy;
	
	//ディフーズカラー設定
	Out.Diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
}
