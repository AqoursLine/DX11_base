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

// 更新用パラメータ
cbuffer UpdateParams : register(b3)
{
	float deltaTime;
	float gravity;
	float startSize;
	float endSize;
	float4 startColor;
	float4 endColor;
}

// パーティクルバッファ
RWStructuredBuffer<Particle> particles : register(u0);

// フリーインデックスバッファ
AppendStructuredBuffer<uint> freeIndices : register(u1);

// パーティクル更新コンピュートシェーダ
[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint index = DTid.x;

	Particle p = particles[index];

	// 非アクティブなパーティクルはスキップ
	if (p.active == 0) return;

	// ライフタイムの更新
	p.life -= deltaTime;
	if (p.life <= 0.0f)
	{
		p.active = 0; // パーティクルを非アクティブにする
		particles[index] = p;
		
		// フリーインデックスバッファにインデックスを追加
		freeIndices.Append(index);
		return;
	}
	
	// 位置更新
	p.position += p.velocity * deltaTime;
	
	// 重力の適用
	p.velocity.y += gravity * deltaTime;
	
	// 回転更新
	p.rotation += p.rotationSpeed * deltaTime;
	
	// 回転を0~2πの範囲に収める
	p.rotation = p.rotation - 6.2831853f * floor(p.rotation / 6.2831853f);
	
	// ライフタイム比率計算
	float lifeRatio = p.life / p.maxLife;
	
	// サイズ補間
	p.size = lerp(endSize, startSize, lifeRatio);
	
	// カラー補間
	p.color = lerp(endColor, startColor, lifeRatio);

	particles[index] = p;
}