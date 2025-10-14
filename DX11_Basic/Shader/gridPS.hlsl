#include "common.hlsl"

Texture2D g_texture0 : register(t0);
SamplerState g_sampler : register(s0);

void main(in PS_INPUT In, out float4 outDiffuse : SV_Target)
{
	float3 worldPos = In.WorldPosition.xyz;
	float3 grid = abs(frac(worldPos) - 0.5) * 2.0;

	float3 lines;
	lines.x = step(0.1, grid.x);
	lines.y = step(0.1, grid.y);
	lines.z = step(0.1, grid.z);
	
	float minLine = lines.x * lines.y * lines.z;

	float4 gridColor = float4(1, 1, 1, 1);
	gridColor.rgb *= minLine;

	outDiffuse = In.Diffuse * Material.Diffuse * gridColor;
}