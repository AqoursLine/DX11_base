#include "common.hlsl"

void main(in PS_INPUT In, out float4 outDiffuse : SV_Target)
{
	//outDiffuse = In.Diffuse * Material.Diffuse;
	
	//for (int i = 0; i < MAX_LIGHTS; i++)
	//{
	//	outDiffuse.rgb *= Lights[i].DiffuseAndRange.rgb;
	//}

	outDiffuse.rgb = Lights[0].DiffuseAndRange.rgb;
	
	outDiffuse.a = 1.0f;
}
