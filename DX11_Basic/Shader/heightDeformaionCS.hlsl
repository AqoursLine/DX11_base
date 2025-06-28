cbuffer DeformParamas : register(b0)
{
	float2 centerUV;
	float radius;
	float depth;
	float padding;
};

RWTexture2D<float> heightMap : register(u0);

[numthreads(8, 8, 1)]
void main ( uint3 id : SV_DispatchThreadID )
{
	uint2 texSize;
	heightMap.GetDimensions(texSize.x, texSize.y);
	
	if (id.x >= texSize.x || id.y >= texSize.y)
		return;
	
	float2 uv = float2(id.xy) / float2(texSize);
	float2 offset = uv - centerUV;
	float dist = length(offset);
	
	if (dist < radius)
	{
		float falloff = 1.0 - (dist / radius);
		falloff = falloff * falloff;
		float deformation = falloff * depth;
		heightMap[id.xy] -= deformation;
	}
}