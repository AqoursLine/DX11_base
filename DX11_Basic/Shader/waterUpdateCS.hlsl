
//================================================================
// 定数定義
//================================================================
#define MAX_RIPPLES 32
#define THREAD_GROUP_SIZE 16
#define GRID_RESOLUTION 512 // グリッドの解像度（512x512）

//================================================================
// 構造体定義
//================================================================

// 頂点データ構造体
struct Vertex
{
	float4 position; // xyz:位置 w:未使用
	float4 normal; // xyz:法線 w:未使用
	float4 diffuse; // rgba:色
	float4 texcoord; // uv:テクスチャ座標
	float4 tangent; // xyz:接線 w:未使用
};

// 波紋データ構造体
struct RippleData
{
	float4 PositionAndTime; // xyz:位置 w:開始時間
	float4 Params; // x:振幅 y:周波数 z:速度 w:使用フラグ（常に1.0）
};

//================================================================
// 定数バッファ
//================================================================
cbuffer WaterConstantBuffer : register(b0)
{
	float Time;
	float WaveHeight;
	float WaterSize;
	int ActiveRippleCount; // アクティブな波紋の数（動的ループ用）

	// 基本波パラメータ
	float BaseWaveFreq1;
	float BaseWaveFreq2;
	float BaseWaveFreq3;
	float BaseWaveSpeed1;
	float BaseWaveSpeed2;
	float BaseWaveSpeed3;
	float WaveSharpness; // 波頭の鋭さ（べき乗の指数）
	float GridResolution;
	
	// グリッド情報
	float CellSize; // グリッドのセルサイズ
	float NormalDelta; // 法線計算用のデルタ値
	float Padding[2]; // パディング
	
	// 波紋データ（先頭から詰まっている）
	RippleData Ripples[MAX_RIPPLES];
}

//================================================================
// UAV定義
//================================================================
RWStructuredBuffer<Vertex> WaterVertexBuffer : register(u0); // 水面頂点バッファ

//================================================================
// ヘルパー関数
//================================================================

// べき乗sin波関数
float ApplySharpness(float wave)
{
	float wave01 = wave * 0.5f + 0.5f;
	wave01 = pow(wave01, WaveSharpness);
	return wave01 * 2.0f - 1.0f;
}

// 基本波の高さ計算
float CalculateBaseWaves(float2 worldPos)
{
	float x = worldPos.x;
	float z = worldPos.y;
	
	// 三つの基本波
	float wave1 = sin(x * BaseWaveFreq1 + Time * BaseWaveSpeed1);
	float wave2 = sin(z * BaseWaveFreq2 + Time * BaseWaveSpeed2);
	float wave3 = sin((x + z) * BaseWaveFreq3 + Time * BaseWaveSpeed3);
	
	// べき乗sin波を適用
	wave1 = ApplySharpness(wave1);
	wave2 = ApplySharpness(wave2);
	wave3 = ApplySharpness(wave3);
	
	// 重みつき合成
	return (wave1 * 0.3f + wave2 * 0.2f + wave3 * 0.5f) * WaveHeight;
}

// 波紋の高さ計算
float CalculateRippleWaves(float2 worldPos)
{
	float height = 0.0f;
	float x = worldPos.x;
	float z = worldPos.y;
	
	// アクティブな波紋をループ
	for (int i = 0; i < ActiveRippleCount; i++)
	{
		RippleData ripple = Ripples[i];
		
		// アクティブフラグチェック
		if (ripple.Params.w < 0.5f)
			continue;
			
		// 波紋の中心位置からの距離
		float dx = x - ripple.PositionAndTime.x;
		float dz = z - ripple.PositionAndTime.z;
		float distance = sqrt(dx * dx + dz * dz);
		
		float rippleTime = ripple.PositionAndTime.w;
		float amplitude = ripple.Params.x;
		float frequency = ripple.Params.y;
		float speed = ripple.Params.z;
		
		// 波紋の広がり範囲内かチェック
		if (distance < speed * rippleTime && rippleTime > 0.0f)
		{
			// 波紋の位相計算
			float wavePhase = frequency * (distance - speed * rippleTime);
			
			// 減衰計算
			float timeAttenuation = exp(-rippleTime * 0.5f);
			float distAttenuation = 1.0f / (1.0f + distance * 0.01f);
			
			// 波紋の高さ寄与
			height += sin(wavePhase) * amplitude * timeAttenuation * distAttenuation;
		}
		
	}

	return height;
}

// 総合的な波高計算
float CalculateWaveHeight(float2 worldPos)
{
	return CalculateBaseWaves(worldPos) + CalculateRippleWaves(worldPos);
}

// 法線計算
float3 CalculateNormal(float2 worldPos)
{
	float delta = NormalDelta;
	
	// 4方向の高さ差分
	float heightL = CalculateWaveHeight(worldPos - float2(delta, 0.0f));
	float heightR = CalculateWaveHeight(worldPos + float2(delta, 0.0f));
	float heightD = CalculateWaveHeight(worldPos - float2(0.0f, delta));
	float heightU = CalculateWaveHeight(worldPos + float2(0.0f, delta));
	
	// 中心差分で勾配計算
	float3 normal;
	normal.x = (heightL - heightR) / (2.0f * delta);
	normal.y = 1.0f;
	normal.z = (heightD - heightU) / (2.0f * delta);
	
	return normalize(normal);
}

// 接線計算
float3 CalculateTangent(float2 worldPos)
{
	float delta = NormalDelta;
	
	// X方向の高さ差分
	float heightL = CalculateWaveHeight(worldPos - float2(delta, 0.0f));
	float heightR = CalculateWaveHeight(worldPos + float2(delta, 0.0f));
	
	// 接線ベクトル
	float3 tangent;
	tangent.x = 1.0f;
	tangent.y = (heightR - heightL) / (2.0f * delta);
	tangent.z = 0.0f;
	
	return normalize(tangent);
}

//================================================================
// コンピュートシェーダーメイン関数
//================================================================
[numthreads(THREAD_GROUP_SIZE, THREAD_GROUP_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	// 範囲チェック
	if (DTid.x >= GRID_RESOLUTION + 1 || DTid.y >= GRID_RESOLUTION + 1)
		return;
		

	// 頂点インデックス計算
	uint vertexIndex = DTid.y * (GRID_RESOLUTION + 1) + DTid.x;
	
	// 現在の頂点データを読み込み
	Vertex vertex = WaterVertexBuffer[vertexIndex];
	
	// ワールド位置計算
	float2 worldPos = float2(vertex.position.x, vertex.position.z);
	
	// 波高計算
	float height = CalculateWaveHeight(worldPos);
	
	// y座標を更新
	vertex.position.y = height;
	
	// 法線計算
	vertex.normal = float4(CalculateNormal(worldPos), 0.0f);
	
	// 接線計算
	vertex.tangent = float4(CalculateTangent(worldPos), 0.0f);
	
	// 更新した頂点データを書き込み
	WaterVertexBuffer[vertexIndex] = vertex;
}
