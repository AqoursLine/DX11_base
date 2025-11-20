#include "main.h"
#include "renderer.h"
#include "shaders.h"
#include "texture.h"
#include "splashEffect.h"

#include "myRandom.h"

/// <summary>
/// コンストラクタ
/// </summary>
SplashEffect::SplashEffect()
	: m_vertexBuffer(nullptr)
	, m_instanceBuffer(nullptr)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_texture(nullptr)
	, m_blendState(nullptr)
	, m_maxParticles(1000)
	, m_minLifeTime(0.5f)
	, m_maxLifeTime(2.0f)
	, m_minSize(0.5f)
	, m_maxSize(2.5f)
	, m_minSpeed(3.0f)
	, m_maxSpeed(10.0f)
	, m_gravity(-9.8f)
{
	m_particles.resize(m_maxParticles);
}

/// <summary>
/// パーティクル発生
/// </summary>
/// <param name="position">座標</param>
/// <param name="direction">方向</param>
/// <param name="intensity">強度</param>
void SplashEffect::EmitSplash(const Vector3& position, const Vector3& direction, float intensity) {
	int particleCount = static_cast<int>(10 * intensity);

	for (int i = 0; i < particleCount; i++) {
		// ランダムな速度ベクトルを生成
		float angle = MyRandom::GetFloat(0.0f, XM_2PI);
		float elevation = MyRandom::GetFloat(0.3f, 1.0f);

		Vector3 velocity = direction;
		velocity.x += std::cos(angle) * elevation;
		velocity.z += std::sin(angle) * elevation;
		velocity.y = std::abs(velocity.y) + MyRandom::GetFloat(0.5f, 2.0f);
		velocity.Normalize();

		velocity *= MyRandom::GetFloat(m_minSpeed, m_maxSpeed) * intensity;

		float size = MyRandom::GetFloat(m_minSize, m_maxSize);
		float lifeTime = MyRandom::GetFloat(m_minLifeTime, m_maxLifeTime);

		EmitParticle(position, velocity, size, lifeTime);		
	}
}

/// <summary>
/// 連続的に少量のパーティクルを発生させる
/// </summary>
/// <param name="position">座標</param>
/// <param name="direction">方向</param>
/// <param name="intensity">強度</param>
void SplashEffect::EmitContinuousSplash(const Vector3& position, const Vector3& direction, float intensity) {
	int particleCount = static_cast<int>(2 * intensity);

	for (int i = 0; i < particleCount; i++) {
		Vector3 velocity = direction;
		velocity.x += MyRandom::GetFloat(-0.3f, 0.3f);
		velocity.z += MyRandom::GetFloat(-0.3f, 0.3f);
		velocity.y = MyRandom::GetFloat(0.5f, 1.5f);
		velocity *= MyRandom::GetFloat(m_minSpeed * 0.5f, m_maxSpeed * 0.5f);

		float size = MyRandom::GetFloat(m_minSize * 0.7f, m_maxSize * 0.7f);
		float lifeTime = MyRandom::GetFloat(m_minLifeTime, m_maxLifeTime * 0.8f);

		EmitParticle(position, velocity, size, lifeTime);
	}
}

/// <summary>
/// 波紋エフェクト発生
/// </summary>
/// <param name="position">座標</param>
/// <param name="waveHeight">波の高さ</param>
void SplashEffect::EmitWaveSplash(const Vector3& position, float waveHeight) {
	float intensity = std::min(waveHeight / 5.0f, 1.0f);
	int particleCount = static_cast<int>(5 * intensity);

	for (int i = 0; i < particleCount; i++) {
		Vector3 velocity;
		velocity.x = MyRandom::GetFloat(-1.0f, 1.0f);
		velocity.z = MyRandom::GetFloat(-1.0f, 1.0f);
		velocity.y = MyRandom::GetFloat(1.0f, 3.0f);
		velocity.Normalize();

		velocity *= MyRandom::GetFloat(m_minSpeed, m_maxSpeed) * intensity;

		float size = MyRandom::GetFloat(m_minSize, m_maxSize);
		float lifeTime = MyRandom::GetFloat(m_minLifeTime, m_maxLifeTime);
		EmitParticle(position, velocity, size, lifeTime);
	}
}

/// <summary>
/// 最大パーティクル数設定
/// </summary>
/// <param name="maxParticles">最大パーティクル数</param>
void SplashEffect::SetMaxParticles(int maxParticles) {
	m_maxParticles = maxParticles;
	m_particles.resize(m_maxParticles);
}

/// <summary>
/// パーティクル寿命設定
/// </summary>
/// <param name="minLifeTime">最小寿命</param>
/// <param name="maxLifeTime">最大寿命</param>
void SplashEffect::SetParticleLifeTime(float minLifeTime, float maxLifeTime) {
	m_minLifeTime = minLifeTime;
	m_maxLifeTime = maxLifeTime;
}

/// <summary>
/// パーティクルサイズ設定
/// </summary>
/// <param name="minSize">最小サイズ</param>
/// <param name="maxSize">最大サイズ</param>
void SplashEffect::SetParticleSize(float minSize, float maxSize) {
	m_minSize = minSize;
	m_maxSize = maxSize;
}

/// <summary>
/// パーティクル速度設定
/// </summary>
/// <param name="minSpeed">最小速度</param>
/// <param name="maxSpeed">最大速度</param>
void SplashEffect::SetParticleSpeed(float minSpeed, float maxSpeed) {
	m_minSpeed = minSpeed;
	m_maxSpeed = maxSpeed;
}

/// <summary>
/// アクティブなパーティクル数取得
/// </summary>
/// <returns>アクティブなパーティクル数</returns>
int SplashEffect::GetActiveParticleCount() const {
	int count = 0;
	for (const auto& particle : m_particles) {
		if (particle.active) {
			count++;
		}
	}
	return count;
}

/// <summary>
/// イニシャライズ
/// </summary>
/// <returns>初期化完了</returns>
bool SplashEffect::Initialize() {
	auto device = RENDERER.GetDevice();

	// シェーダー読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\splashEffectVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\splashEffectPS.cso");

	// テクスチャ読み込み
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\circle.png")) {
		return false;
	}

	// 頂点バッファ生成
	VERTEX_3D vertices[4] = {
		{ XMFLOAT4(-0.5f, 0.5f, 0.0f, 1.0f), XMFLOAT4(0,0,0,0), XMFLOAT4(0,1,0,0), XMFLOAT4(0,0,0,0), XMFLOAT4(1,1,1,1) },
		{ XMFLOAT4(0.5f, 0.5f, 0.0f, 1.0f), XMFLOAT4(0,0,0,0), XMFLOAT4(1,1,0,0), XMFLOAT4(0,0,0,0), XMFLOAT4(1,1,1,1) },
		{ XMFLOAT4(-0.5f, -0.5f, 0.0f, 1.0f), XMFLOAT4(0,0,0,0), XMFLOAT4(0,0,0,0), XMFLOAT4(0,0,0,0), XMFLOAT4(1,1,1,1) },
		{ XMFLOAT4(0.5f, -0.5f, 0.0f, 1.0f), XMFLOAT4(0,0,0,0), XMFLOAT4(1,0,0,0), XMFLOAT4(0,0,0,0), XMFLOAT4(1,1,1,1) },
	};

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = sizeof(vertices);
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertices;
	HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(hr)) {
		ErrorMessage(L"スプラッシュエフェクトの頂点バッファの作成に失敗しました。", hr);
		return false;
	}

	// インスタンスバッファ生成
	D3D11_BUFFER_DESC instanceBufferDesc = {};
	instanceBufferDesc.ByteWidth = sizeof(ParticleVertex) * m_maxParticles;
	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = device->CreateBuffer(&instanceBufferDesc, nullptr, &m_instanceBuffer);
	if (FAILED(hr)) {
		ErrorMessage(L"スプラッシュエフェクトのインスタンスバッファの作成に失敗しました。", hr);
		return false;
	}

	// ブレンドステート生成
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = device->CreateBlendState(&blendDesc, &m_blendState);
	if (FAILED(hr)) {
		ErrorMessage(L"スプラッシュエフェクトのブレンドステートの作成に失敗しました。", hr);
		return false;
	}

	// パーティクル初期化
	for (auto& particle : m_particles) {
		particle.active = false;
	}
	return true;
}

/// <summary>
/// 終了
/// </summary>
void SplashEffect::Finalize() {
	if (m_vertexBuffer) {
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
	if (m_instanceBuffer) {
		m_instanceBuffer->Release();
		m_instanceBuffer = nullptr;
	}
	if (m_blendState) {
		m_blendState->Release();
		m_blendState = nullptr;
	}
	if (m_vertexShader) {
		delete m_vertexShader;
		m_vertexShader = nullptr;
	}
	if (m_pixelShader) {
		delete m_pixelShader;
		m_pixelShader = nullptr;
	}
	if (m_texture) {
		delete m_texture;
		m_texture = nullptr;
	}
}

/// <summary>
/// 更新
/// </summary>
/// <param name="deltaTime">デルタタイム</param>
void SplashEffect::Update(double deltaTime) {
	float dt = static_cast<float>(deltaTime);

	// パーティクル更新
	for (auto& particle : m_particles) {
		if (!particle.active) continue;
		
		// 寿命更新
		particle.life -= dt;
		if (particle.life <= 0.0f) {
			particle.active = false;
			continue;
		}

		// 重力影響
		particle.velocity.y += m_gravity * dt;

		// 位置更新
		particle.position += particle.velocity * dt;

		// 回転更新
		particle.rotation += particle.angularVelocity * dt;
	}
}

/// <summary>
/// 描画
/// </summary>
void SplashEffect::Draw() const {
	auto context = RENDERER.GetDeviceContext();

	// インスタンスバッファ更新
	const_cast<SplashEffect*>(this)->UpdateInstanceBuffer();

	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// テクスチャ設定
	m_texture->Set(0);

	// ブレンドステート設定
	float blendFactor[4] = { 0.0f,0.0f,0.0f,0.0f };
	context->OMSetBlendState(m_blendState, blendFactor, 0xffffffff);

	// 頂点バッファ設定
	ID3D11Buffer* buffers[2] = { m_vertexBuffer, m_instanceBuffer };
	UINT strides[2] = { sizeof(VERTEX_3D), sizeof(ParticleVertex) };
	UINT offsets[2] = { 0, 0 };
	context->IASetVertexBuffers(0, 2, buffers, strides, offsets);

	// プリミティブトポロジー設定
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// アクティブなパーティクル数をカウント
	int activeCount = 0;
	for (const auto& particle : m_particles) {
		if (particle.active) {
			++activeCount;
		}
	}

	// インスタンス描画
	if (activeCount > 0) {
		context->DrawInstanced(4, activeCount, 0, 0);
	}

	// ブレンドステート解除
	RENDERER.SetATCEnable(false);
}

/// <summary>
/// インスタンスバッファの更新
/// </summary>
void SplashEffect::UpdateInstanceBuffer() {
	auto context = RENDERER.GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = context->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (SUCCEEDED(hr)) {
		ParticleVertex* vertices = static_cast<ParticleVertex*>(mappedResource.pData);
		int index = 0;

		for (const auto& particle : m_particles) {
			if (!particle.active) continue;
			
			// ライフによる透明度計算
			float alpha = particle.life / particle.maxLife;
			alpha = std::min(alpha * 2.0f, 1.0f); // フェードイン効果

			vertices[index].position = XMFLOAT4(particle.position.x, particle.position.y, particle.position.z, particle.size);
			vertices[index].color = XMFLOAT4(1.0f, 1.0f, 1.0f, alpha);
			vertices[index].rotation.x = particle.rotation.x;
			vertices[index].rotation.y = particle.rotation.y;
			vertices[index].rotation.z = particle.rotation.z;
			++index;
		}

		context->Unmap(m_instanceBuffer, 0);
	}
}

/// <summary>
/// パーティクル発生
/// </summary>
/// <param name="position">座標</param>
/// <param name="velocity">速度</param>
/// <param name="size">サイズ</param>
/// <param name="lifeTime">寿命</param>
void SplashEffect::EmitParticle(const Vector3& position, const Vector3& velocity, float size, float lifeTime) {
	//非アクティブなパーティクルを検索
	for (auto& particle : m_particles) {
		if (!particle.active) {
			particle.position = position;
			particle.velocity = velocity;
			particle.size = size;
			particle.life = lifeTime;
			particle.maxLife = lifeTime;
			particle.rotation = Vector3::ZERO;
			particle.angularVelocity = Vector3(
				MyRandom::GetFloat(-XM_PI, XM_PI),
				MyRandom::GetFloat(-XM_PI, XM_PI),
				MyRandom::GetFloat(-XM_PI, XM_PI)
			);
			particle.active = true;
			break;
		}
	}
}
