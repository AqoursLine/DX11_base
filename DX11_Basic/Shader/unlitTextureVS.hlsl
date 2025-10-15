#include "common.hlsl"

void main( in VS_INPUT In, out PS_INPUT Out )
{
	//ワールドビュープロジェクション行列を計算
	matrix wvp = mul(WorldMatrix, ViewMatrix);
	wvp = mul(wvp, ProjectionMatrix);
	
	//頂点座標をクリップ空間に変換
	Out.Position = mul(In.Position, wvp);
	
	//頂点をワールド空間に変換
	Out.WorldPosition = mul(In.Position, WorldMatrix);

	//法線の向きを回転
	float4 worldNormal, normal;
	normal = float4(In.Normal.xyz, 0);
	worldNormal = mul(normal, WorldMatrix);
	worldNormal = normalize(worldNormal);
	Out.Normal = worldNormal;
	
	//UV座標のxy成分をそのまま渡す
	Out.TexCoord = In.TexCoord.xy;
	
	//頂点カラーをそのまま渡す
	Out.Diffuse = In.Diffuse;
}