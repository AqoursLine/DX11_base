// 波紋データ構造体
struct RippleData
{
	float4 PositionAndTime; // xyz:位置, w:時間
	float4 Params;			// x:振幅, y:周波数, z:速度, w:使用フラグ
};

#define MAX_RPPLE_NUM 32

// コンピュートシェーダー用定数バッファ
cbuffer WaterComputeBuffer : register(b7)
{
	float Time; // 経過時間
	float WaveHeight; // 波の高さ
	float WaterSize; // 水面のサイズ
	int ActiveRippleCount; // アクティブな波紋の数
	
	float BaseWaveFreq1; // 基本波1の周波数
	float BaseWaveFreq2; // 基本波2の周波数
	float BaseWaveFreq3; // 基本波3の周波数
	float BaseWaveSpeed1; // 基本波1の速度
	float BaseWaveSpeed2; // 基本波2の速度
	float BaseWaveSpeed3; // 基本波3の速度
	float WaveSharpness; // 波の鋭さ
	int GridResolution; // グリッド解像度
	
	RippleData Ripples[MAX_RPPLE_NUM]; // 波紋データ配列
}

// 出力テクスチャ
RWTexture2D<float4> OutputHeightNormal : register(u0);

// Gerstner波計算用関数
float CalculateGerstnerWave(float2 pos, float frequency, float speed, float time, float sharpness)
{
	float k = frequency * 6.28318530718; // 波数
	float c = speed; // 波の速度
	float a = 1.0 / (k * sharpness); // 振幅

	float2 d = normalize(float2(1.0, 0.3)); // 波の進行方向
	float phase = k * dot(d, pos) - c * time; // 位相

	return a * pow(sin(phase) * 0.5 + 0.5, sharpness); // 波の高さ
}

// 波紋の計算
float CalculateRipple(float2 pos, RippleData ripple)
{
	float3 ripplePos = ripple.PositionAndTime.xyz;
	float rippleTime = ripple.PositionAndTime.w;
	float amplitude = ripple.Params.x;
	float frequency = ripple.Params.y;
	float speed = ripple.Params.z;
	float active = ripple.Params.w;
	if (active < 0.5)
		return 0.0;
	
	float2 diff = pos - ripplePos.xz;
	float dist = length(diff);
	float radius = speed * rippleTime;
	
	// 減衰
	float attenuation = exp(-rippleTime * 0.5);

	// 波の高さ計算
	float wave = sin(dist * frequency - rippleTime * 3.0) * attenuation;
	
	// 距離による影響範囲
	float distFade = smoothstep(radius + 5.0, radius, dist);
	
	return wave * amplitude * distFade * attenuation;
}

// 指定位置の高さを計算
float CalculateHeight(float2 pos)
{
	float height = 0.0;
	// 基本波の合成
	height += CalculateGerstnerWave(pos, BaseWaveFreq1, BaseWaveSpeed1, Time, WaveSharpness);
	height += CalculateGerstnerWave(pos * 0.7, BaseWaveFreq2, BaseWaveSpeed2, Time * 1.3, WaveSharpness);
	height += CalculateGerstnerWave(pos * 1.3, BaseWaveFreq3, BaseWaveSpeed3, Time * 0.8, WaveSharpness);

	// 波紋の影響を加算
	for (int i = 0; i < ActiveRippleCount; i++)
	{
		height += CalculateRipple(pos, Ripples[i]);
	}
	return height;
}

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	// グリッドの範囲外チェック
	if (DTid.x >= GridResolution || DTid.y >= GridResolution)
		return;
	
	// uv座標からワールド座標に変換
	float2 uv = float2(DTid.x, DTid.y) / (GridResolution - 1);
	float2 worldPos = (uv - 0.5) * WaterSize;
	
	// 中心点の高さを計算
	float height = CalculateHeight(worldPos);
	
	// 法線計算のための隣接点の高さを取得
	float delta = WaterSize / (GridResolution - 1);
	
	float heightR = CalculateHeight(worldPos + float2(delta, 0.0));
	float heightL = CalculateHeight(worldPos - float2(delta, 0.0));
	float heightU = CalculateHeight(worldPos + float2(0.0, delta));
	float heightD = CalculateHeight(worldPos - float2(0.0, delta));
	
	// 勾配から法線を計算
	float3 tangentX = float3(delta * 2.0, (heightR - heightL) * WaveHeight, 0.0);
	float3 tangentZ = float3(0.0, (heightU - heightD) * WaveHeight, delta * 2.0);

	float3 normal = normalize(cross(tangentX, tangentZ));

	// 出力テクスチャに高さと法線を格納
	OutputHeightNormal[DTid.xy] = float4(normal, height);


}