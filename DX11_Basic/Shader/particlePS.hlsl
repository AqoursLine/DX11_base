Texture2D g_particleTexture : register(t1);
SamplerState g_particleSampler : register(s0);

struct PS_PARTICLE_INPUT
{
	float4 Position : SV_POSITION; // クリップ空間位置
	float2 TexCoord : TEXCOORD0; // テクスチャ座標
	float4 Color : COLOR0; // 頂点カラー
};

void main(in PS_PARTICLE_INPUT inputm, out float4 outputColor : SV_TARGET)
{
	// テクスチャカラーを取得
	float4 texColor = g_particleTexture.Sample(g_particleSampler, inputm.TexCoord);
	
	// 頂点カラーとテクスチャカラーを乗算
	outputColor = texColor * inputm.Color;
	
}
