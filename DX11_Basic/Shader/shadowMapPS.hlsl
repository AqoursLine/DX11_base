#include "common.hlsl"

struct VS_SHADOW_OUTPUT
{
	float4 position : SV_POSITION;
};

// ピクセルシェーダー
// 深度値を書き込むだけ
void main(VS_SHADOW_OUTPUT input)
{
}
