#include "common.hlsl"

struct VS_PARTICLE_INPUT
{
	float2 Offset : OFFSET;
	float2 TexCoord : TEXCOORD;
	
	// インスタンスデータ
	float3 InstancePosition : INSTANCE_POSITION;
	float InstanceSize : INSTANCE_SIZE;
	float4 InstanceColor : INSTANCE_COLOR;
	float2 InstanceTexOffset : INSTANCE_TEXOFFSET;
};

struct PS_PARTICLE_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float4 Diffuse : COLOR;
};

void main(in VS_PARTICLE_INPUT input, out PS_PARTICLE_INPUT output)
{
	// ビルボード用の頂点位置を計算
	float3 worldPos = input.InstancePosition 
					+ CameraRight.xyz * input.Offset.x * input.InstanceSize
					+ CameraUp.xyz * input.Offset.y * input.InstanceSize;

	float4 viewPos = mul(float4(worldPos, 1.0f), ViewMatrix);
	output.Position = mul(viewPos, ProjectionMatrix);
	
	output.TexCoord = input.TexCoord + input.InstanceTexOffset;
	output.Diffuse = input.InstanceColor;
}
