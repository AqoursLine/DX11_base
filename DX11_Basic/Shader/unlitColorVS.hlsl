#include "common.hlsl"

void main(in VS_INPUT In, out PS_INPUT Out)
{
	//ワールドビュープロジェクション
	matrix wvp;
	wvp = mul(WorldMatrix, ViewMatrix);
	wvp = mul(wvp, ProjectionMatrix);
	
	//頂点座標を行列で変換
	Out.Position = mul(In.Position, wvp);
	
	//頂点座標をワールド行列で変換
	Out.WorldPosition = mul(In.Position, WorldMatrix);
	
	//法線をそのまま渡す
	Out.Normal = In.Normal;
	
	//頂点カラーをそのまま渡す
	Out.Diffuse = In.Diffuse;
	
	//テクスチャ座標をそのまま渡す
	Out.TexCoord = In.TexCoord;
}