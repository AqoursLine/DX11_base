#include "common.hlsl"

// ヘルパー関数
float RoundedBox(float2 p, float2 size, float radius)
{
	float2 d = abs(p) - size + radius;
	return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0f) - radius;
}

float smoothAlpha(float dist, float smoothness)
{
	return 1.0f - smoothstep(-smoothness, smoothness, dist);
}

void main(in PS_INPUT input, out float4 output : SV_TARGET)
{
	float cornerRadius = params1.x;	// 角の半径
	float smoothness = params1.y;	// エッジの滑らかさ
	float2 rectSize = params1.zw;	// 長方形のサイズ（半分）

	// UV座標を-0.5～0.5に変換
	float2 uv = input.TexCoord.xy - float2(0.5f, 0.5f);
	
	// アスペクト比を考慮してスケーリング
	float2 size = rectSize * 0.5f;
	
	// 角丸四角形のSDFを計算
	float dist = RoundedBox(uv, size, cornerRadius);
	
	// 滑らかなアルファ値を計算
	float alpha = smoothAlpha(dist, smoothness);
	
	// 最終色を設定
	output = Material.Diffuse * input.Diffuse;
	output.a *= alpha;

}
