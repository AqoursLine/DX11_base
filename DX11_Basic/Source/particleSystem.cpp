#include "main.h"
#include "renderer.h"
#include "shaders.h"
#include "particleSystem.h"

#include "myRandom.h"

void ParticleSystem::Emit(int count) {
	for (int i = 0; i < count; i++) {
		// 非アクティブなパーティクルを探す
		for (auto& particle : m_particles) {
			if (particle.active) continue;

			// パーティクル初期化
			Vector3 offsetPos = Vector3(
				MyRandom::GetFloat(-1.0f, 1.0f) * m_settings.position.x,
				MyRandom::GetFloat(-1.0f, 1.0f) * m_settings.position.y,
				MyRandom::GetFloat(-1.0f, 1.0f) * m_settings.position.z
			);
			particle.position = m_position + offsetPos;

			// 速度にランダムな変動を加える
			particle.velocity = m_settings.velocity + Vector3(
				MyRandom::GetFloat(-1.0f, 1.0f) * m_settings.velocityVariation.x,
				MyRandom::GetFloat(-1.0f, 1.0f) * m_settings.velocityVariation.y,
				MyRandom::GetFloat(-1.0f, 1.0f) * m_settings.velocityVariation.z
			);

			particle.color = m_settings.startColor;
			particle.size = m_settings.startSize;
			particle.life = m_settings.lifeTime;
			particle.maxLife = m_settings.lifeTime;
			particle.rotation = 0.0f;
			particle.active = true;

			break;
		}
	}
}

void ParticleSystem::EmitOneShot(const Vector3& position, int count) {
	Vector3 originalPosition = m_position;
	m_position = position;

	if (count < 0) {
		count = m_settings.oneShotCount;
	}

	Emit(count);
	m_position = originalPosition;
}

bool ParticleSystem::Initialize() {
	// パーティクル配列の初期化
	m_particles.resize(m_settings.maxParticles);
	for (auto& particle : m_particles) {
		particle.active = false;
	}

	// バッファ、シェーダー、ステートの作成
	if (!CreateBuffers()) return false;
	if (!CreateShaders()) return false;
	if (!CreateStates()) return false;

	return true;
}

void ParticleSystem::Finalize() {
	// リソース解放
	if (m_vertexBuffer) { m_vertexBuffer->Release(); m_vertexBuffer = nullptr; }
	if (m_indexBuffer) { m_indexBuffer->Release(); m_indexBuffer = nullptr; }
	if (m_instanceBuffer) { m_instanceBuffer->Release(); m_instanceBuffer = nullptr; }
	if (m_vertexShader) { delete m_vertexShader; m_vertexShader = nullptr; }
	if (m_pixelShader) { delete m_pixelShader; m_pixelShader = nullptr; }
	if (m_blendState) { m_blendState->Release(); m_blendState = nullptr; }
}

void ParticleSystem::Update(double deltaTime) {
	float dt = static_cast<float>(deltaTime);

	if (m_isPaused) return;

	// パーティクル発生
	if (m_isPlaying && !m_settings.oneShot) {
		m_enmitTimer += dt;
		float emitInterval = 1.0f / m_settings.emitRate;

		while (m_enmitTimer >= emitInterval) {
			Emit(1);
			m_enmitTimer -= emitInterval;
		}
	}

	// パーティクル更新
	UpdateParticles(dt);
}

void ParticleSystem::Draw() const {
	if (!m_textureSRV) return;

	auto context = RENDERER.GetDeviceContext();

	// アクティブなパーティクル数をカウント
	std::vector<ParticleInstance> instances;
	instances.reserve(m_settings.maxParticles);

	for (const auto& particle : m_particles) {
		if (!particle.active) continue;

		ParticleInstance instance;
		instance.position = XMFLOAT3(particle.position.x, particle.position.y, particle.position.z);
		instance.size = particle.size;
		instance.color = XMFLOAT4(particle.color.x, particle.color.y, particle.color.z, particle.color.w);
		// テクスチャオフセット計算（例として単純に0に設定）
		instance.texOffset = XMFLOAT2(0.0f, 0.0f);

		instances.push_back(instance);
	}

	if (instances.empty()) return;

	// インスタンスバッファ更新
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, instances.data(), sizeof(ParticleInstance) * instances.size());
	context->Unmap(m_instanceBuffer, 0);

	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// テクスチャ設定
	context->PSSetShaderResources(0, 1, &m_textureSRV);

	// ブレンドステート設定
	float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	context->OMSetBlendState(m_blendState, blendFactor, 0xffffffff);

	// 頂点バッファとインスタンスバッファ設定
	UINT stride[2] = { sizeof(BillboardVertex), sizeof(ParticleInstance) };
	UINT offset[2] = { 0, 0 };
	ID3D11Buffer* buffers[2] = { m_vertexBuffer, m_instanceBuffer };

	context->IASetVertexBuffers(0, 2, buffers, stride, offset);
	context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// プリミティブトポロジー設定
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// インスタンス数分描画
	context->DrawIndexedInstanced(6, static_cast<UINT>(instances.size()), 0, 0, 0);

	// ステートリセット
	context->OMSetBlendState(nullptr, nullptr, 0xffffffff);

}

void ParticleSystem::UpdateParticles(float deltaTime) {
	for (auto& particle : m_particles) {
		if (!particle.active) continue;
		
		// 寿命を減らす
		particle.life -= deltaTime;
		if (particle.life <= 0.0f) {
			particle.active = false;
			continue;
		}

		// 位置更新
		particle.position += particle.velocity * deltaTime;

		// 重力適用
		particle.velocity.y += m_settings.gravity * deltaTime;

		// ライフタイム比率計算
		float lifeRatio = particle.life / particle.maxLife;

		// サイズ補間
		particle.size = m_settings.startSize * lifeRatio + m_settings.endSize * (1.0f - lifeRatio);

		// カラー補間
		particle.color = m_settings.startColor * lifeRatio + m_settings.endColor * (1.0f - lifeRatio);
	}
}

bool ParticleSystem::CreateBuffers() {
	auto device = RENDERER.GetDevice();

	// ビルボード用頂点データ(四角形)
	BillboardVertex vertices[] = {
		{XMFLOAT2(-1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f)},// 左下
		{XMFLOAT2(-1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},// 左上
		{XMFLOAT2(1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f)},// 右下
		{XMFLOAT2(1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)}   // 右上
	};

	// 頂点バッファ作成
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = sizeof(vertices);
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;

	if (FAILED(device->CreateBuffer(&vbDesc, &vbData, &m_vertexBuffer))) {
		return false;
	}

	// インデックスデータ
	UINT indices[] = {
		0, 1, 2,
		2, 1, 3
	};

	// インデックスバッファ作成
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.ByteWidth = sizeof(indices);
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;

	if (FAILED(device->CreateBuffer(&ibDesc, &ibData, &m_indexBuffer))) {
		return false;
	}

	// インスタンスバッファ作成
	D3D11_BUFFER_DESC instDesc = {};
	instDesc.ByteWidth = sizeof(ParticleInstance) * m_settings.maxParticles;
	instDesc.Usage = D3D11_USAGE_DYNAMIC;
	instDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(device->CreateBuffer(&instDesc, nullptr, &m_instanceBuffer))) {
		return false;
	}

	return true;

}

bool ParticleSystem::CreateShaders() {
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader/particleVS.cso");

	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader/particlePS.cso");

	return true;
}

bool ParticleSystem::CreateStates() {
	auto device = RENDERER.GetDevice();

	// 加算合成ブレンドステート作成
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	if (FAILED(device->CreateBlendState(&blendDesc, &m_blendState))) {
		return false;
	}

	return true;
}
