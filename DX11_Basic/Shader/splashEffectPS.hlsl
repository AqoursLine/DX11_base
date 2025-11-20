Texture2D ParticleTexture : register(t0);
SamplerState ParticleSampler : register(s0);

struct PS_PARTICLE_INPUT
{
	float4 Position : SV_POSITION; // クリップ空間位置
	float2 TexCoord : TEXCOORD0; // テクスチャ座標
	float4 Color : COLOR0; // 頂点カラー
};

void main(in PS_PARTICLE_INPUT inputm, out float4 outputColor : SV_TARGET)
{
	// テクスチャカラーを取得
	float4 texColor = ParticleTexture.Sample(ParticleSampler, inputm.TexCoord);
	
	// 頂点カラーとテクスチャカラーを乗算
	outputColor = texColor * inputm.Color;
	
	// アルファ値で透明度を調整
	if (outputColor.a < 0.01f)
	{
		discard; // 透明なピクセルは描画しない
	}
}
