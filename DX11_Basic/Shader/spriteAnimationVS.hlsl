#include "common.hlsl"


void main(in VS_INPUT In, out PS_INPUT Out)
{
	//ワールドビュープロジェクション
	matrix wvp;
	wvp = mul(WorldMatrix, ViewMatrix);
	wvp = mul(wvp, ProjectionMatrix);

	//頂点座標を行列で変換
	Out.Position = mul(In.Position, wvp);
	Out.WorldPosition = mul(In.Position, WorldMatrix);

	//法線の向きを回転
	float4 worldNormal, normal;
	normal = float4(In.Normal.xyz, 0);
	worldNormal = mul(normal, WorldMatrix);
	worldNormal = normalize(worldNormal);
	Out.Normal = worldNormal;
	
	//頂点カラー
	Out.Diffuse = In.Diffuse;

	//テクスチャ座標(パラメータからuvを計算)
	Out.TexCoord = params1.xy + In.TexCoord.xy * params1.zw;
}