// パーティクル構造体
struct Particle
{
	float3 position;
	float3 velocity;
	float4 color;
	float size;
	float life;
	float maxLife;
	float rotation;
	float rotationSpeed;
	uint active;
};

StructuredBuffer<Particle> g_particles : register(t0);
AppendStructuredBuffer<uint> g_activeIndices : register(u0);
RWStructuredBuffer<uint> g_drawArgs : register(u1);

cbuffer CompactParams : register(b3)
{
	uint maxParticles;
	float3 padding;
}

[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint index = DTid.x;
	
	if (index >= maxParticles)
		return;
	
	// スレッド0が引数バッファを初期化
	if (index == 0)
	{
		g_drawArgs[0] = 6; // インデックス数（四角形2つ分）
		// インスタンス数は後で設定
		g_drawArgs[2] = 0; // スタートインデックス
		g_drawArgs[3] = 0; // ベース頂点
		g_drawArgs[4] = 0; // スタートインスタンス
	}
	
	GroupMemoryBarrierWithGroupSync();
	
	// アクティブなパーティクルをチェック
	Particle p = g_particles[index];
	if (p.active != 0)
	{
		g_activeIndices.Append(index);
	}

}