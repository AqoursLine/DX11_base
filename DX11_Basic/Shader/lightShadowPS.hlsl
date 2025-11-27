#include "common.hlsl"
#include "shadowMap.hlsl"

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

void main(PS_INPUT input, out float4 outDiffuse : SV_TARGET)
{
	float4 normal = normalize(input.Normal);
	
	outDiffuse = g_texture.Sample(g_sampler, input.TexCoord);

	float shadowAmount = 1.0;

	for (int i = 0; i < ShadowLightCount; i++)
	{
		float shadow = CalculateHardShadowWithNormalBias(input.WorldPosition.xyz, normal.xyz, i, 0.005);

		shadowAmount *= shadow;

	}

	outDiffuse.rgb *= shadowAmount;

}
