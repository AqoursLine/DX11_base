#include "common.hlsl"

struct VS_SHADOW_OUTPUT
{
	float4 position : SV_POSITION;
};

VS_SHADOW_OUTPUT main(VS_INPUT input)
{
	VS_SHADOW_OUTPUT output;

	// wvp matrix
	matrix worldViewProj = mul(WorldMatrix, ViewMatrix);
	worldViewProj = mul(worldViewProj, ProjectionMatrix);
	
	float4 worldPosition = mul(input.Position, worldViewProj);
	
	output.position = worldPosition;
	
	return output;
}
