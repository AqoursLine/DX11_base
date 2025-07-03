#include "common.hlsl"

void main(in VS_INPUT In, out PS_INPUT Out)
{
	//ワールドビュープロジェクション
	matrix wvp;
	
	//頂点変換
	wvp = mul(WorldMatrix, ViewMatrix);
	//wvp * Projection
	wvp = mul(wvp, ProjectionMatrix);

	//頂点座標を行列で変換
	Out.Position = mul(In.Position, wvp);

	//テクスチャ座標
	Out.TexCoord = In.TexCoord;

	//法線の向きを回転
	float4 worldNormal, normal;
	normal = float4(In.Normal.xyz, 0);
	worldNormal = mul(normal, WorldMatrix);
	worldNormal = normalize(worldNormal);
	Out.Normal = worldNormal;
	
	Out.Diffuse = In.Diffuse;
	
	//ワールド変換した頂点座標を出力
	Out.WorldPosition = mul(In.Position, WorldMatrix);
}