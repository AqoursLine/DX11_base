#include "main.h"
#include "renderer.h"
#include "shaders.h"
#include "splashEffect.h"

/// <summary>
/// コンストラクタ
/// </summary>
SplashEffect::SplashEffect()
	: m_vertexBuffer(nullptr)
	, m_instanceBuffer(nullptr)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
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
			++index;
		}

		context->Unmap(m_instanceBuffer, 0);
	}
}
