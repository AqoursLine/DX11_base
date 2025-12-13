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

// 静的更新用パラメータ
cbuffer StaticUpdateParams : register(b3)
{
	float3 worldAcceleration;
	float startSize;
	float3 localAcceleration;
	float endSize;
	float4 startColor;
	float4 endColor;
}

// 動的更新用パラメータ
cbuffer DynamicUpdateParams : register(b4)
{
	float deltaTime;
	float3 dynamicUpdatePadding;
}

// パーティクルバッファ
RWStructuredBuffer<Particle> particles : register(u0);

// フリーインデックスバッファ
AppendStructuredBuffer<uint> freeIndices : register(u1);

// ローカル座標加速度をワールド座標に変換
float3 ApplyLocalAcceleration(float3 velocity, float3 localAccel)
{
	//速度の大きさが0の場合、方向が定義できないため、ワールド加速度として返す
	float speed = length(velocity);
	if (speed < 0.001f)
		return float3(0, 0, 0);
	
	// forwardベクトルを計算
	float3 forward = velocity / speed;
	
	// up方向をとりあえずY軸に設定
	float3 up = float3(0.0f, 1.0f, 0.0f);
	if (abs(dot(forward, up)) > 0.99f)
	{
		// forwardがY軸に近い場合はZ軸をupに使う
		up = float3(0.0f, 0.0f, 1.0f);
	}
	
	// rightベクトルを計算
	float3 right = normalize(cross(up, forward));
	
	// 再度upベクトルを計算
	up = normalize(cross(forward, right));
	
	// ローカル加速度をワールド座標に変換
	return localAccel.x * right + localAccel.y * up + localAccel.z * forward;
}

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
	
	// 加速度計算
	// ワールド加速度を適用
	p.velocity += worldAcceleration * deltaTime;
	
	// ローカル加速度をワールド座標に変換して適用
	p.velocity += ApplyLocalAcceleration(p.velocity, localAcceleration) * deltaTime;
	
	// 位置更新
	p.position += p.velocity * deltaTime;
	
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