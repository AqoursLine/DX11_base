#include "common.hlsl"

void main(in PS_INPUT In, out float4 outDiffuse : SV_Target)
{
	//入力されたピクセル色をそのまま出力
	outDiffuse = In.Diffuse;
}