Texture2D<float> HeightMap : register(t0);
RWTexture2D<float4> NormalMap : register(u0);

[numthreads(8, 8, 1)]
void main ( uint3 id : SV_DispatchThreadID )
{
	uint2 texSize;
	NormalMap.GetDimensions(texSize.x, texSize.y);
	
	if (id.x >= texSize.x || id.y >= texSize.y)
		return;
	
	float heightScale = 10.0f;
	float2 texelScale = 1.0f / float2(texSize);
	
	float hL = HeightMap[id.xy + uint2(-1, 0)];
	float hR = HeightMap[id.xy + uint2(1, 0)];
	float hD = HeightMap[id.xy + uint2(0, -1)];
	float hU = HeightMap[id.xy + uint2(0, 1)];
	
	float3 normal;
	normal.x = (hL - hR) * heightScale;
	normal.y = (hD - hU) * heightScale;
	normal.z = 2.0f;
	normal = normalize(normal);
	
	NormalMap[id.xy] = float4(normal * 0.5f + 0.5f, 1.0f);
}