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

// 静的発生用パラメータ
cbuffer StaticEmitParams : register(b3)
{
	float emissionAngle;
	float3 baseVelocity;
	float emissionAngleVariation;
	float3 positionVariation;
	float lifeTime;
	float3 velocityVariation;
	float4 startColor;
	float startSize;
	float rotationSpeed;
	float rotationSpeedMin;
	float rotationSpeedMax;
	uint maxParticles;
	float3 emitPadding;
}

// 動的発生用パラメータ
cbuffer DynamicEmitParams : register(b4)
{
	float3 emitPosition;
	uint randomSeed;
	uint emitCount;
	uint3 dynamicEmitPadding;
}

// パーティクルバッファ
RWStructuredBuffer<Particle> particles : register(u0);

// フリーインデックスバッファ
ConsumeStructuredBuffer<uint> freeIndices : register(u1);

// 発生位置バッファ
Buffer<float4> emitPositions : register(t2);

// 位置インデックスバッファ
Buffer<uint> positionIndices : register(t3);

// 乱数生成関数
float random(uint seed, uint index)
{
	uint n = seed + index * 747796405u + 2891336453u;
	n = ((n >> ((n >> 28u) + 4u)) ^ n) * 277803737u;
	return float((n >> 22u) ^ n) / 4294967295.0f;
}

float randomRange(uint seed, uint index, float minVal, float maxVal)
{
	return minVal + (maxVal - minVal) * random(seed, index);
}

// パーティクル発生コンピュートシェーダ
[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint threadIndex = DTid.x;
	
	// emit数を超えたら終了
	if (threadIndex >= emitCount)
		return;
	
	// フリーリストからインデックスを取得
	uint index = freeIndices.Consume();
	
	// 発生位置の取得
	uint posIndex = positionIndices[threadIndex];
	float4 emitPosData = emitPositions[posIndex];
	
	// パーティクルの初期化
	Particle p;
			
	uint baseSeed = randomSeed + threadIndex * 1234567u;
			
			// 位置にランダムなオフセットを加える
	float3 offsetPos = float3(
				randomRange(baseSeed, 0, -1.0f, 1.0f) * positionVariation.x,
				randomRange(baseSeed, 1, -1.0f, 1.0f) * positionVariation.y,
				randomRange(baseSeed, 2, -1.0f, 1.0f) * positionVariation.z
			);
	p.position = emitPosData.xyz + offsetPos;
			
			// 速度の計算
	p.velocity = baseVelocity + float3(
				randomRange(baseSeed, 3, -1.0f, 1.0f) * velocityVariation.x,
				randomRange(baseSeed, 4, -1.0f, 1.0f) * velocityVariation.y,
				randomRange(baseSeed, 5, -1.0f, 1.0f) * velocityVariation.z
			);
			
			// ランダムな方向に速度を回転させる
	p.rotation = emissionAngle + randomRange(baseSeed, 6, -emissionAngleVariation, emissionAngleVariation);
			
			// 回転速度のバラツキ
	p.rotationSpeed = rotationSpeed + randomRange(baseSeed, 7, rotationSpeedMin, rotationSpeedMax);

	p.color = startColor;
	p.size = startSize;
	p.life = lifeTime;
	p.maxLife = lifeTime;
	p.active = 1;

	particles[index] = p;
}