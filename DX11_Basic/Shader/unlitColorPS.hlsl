#include "common.hlsl"

void main (in PS_INPUT In, out float4 outDiffuse : SV_TARGET)
{
	outDiffuse = In.Diffuse;
}