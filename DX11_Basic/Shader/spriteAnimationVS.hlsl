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
	uint gridX = (uint) params1.x; //グリッド数X
	uint gridY = (uint) params1.y; //グリッド数Y
	uint frame = (uint) params1.z; //フレーム番号
	
	//グリッド内の位置を計算
	uint cellX = frame % gridX;
	uint cellY = frame / gridX;

	//セルサイズを計算
	float2 cellSize = float2(1.0f / gridX, 1.0f / gridY);
	
	//テクスチャ座標を計算
	float2 offset = float2(cellX, cellY) * cellSize;
	Out.TexCoord = offset + In.TexCoord * cellSize;
}