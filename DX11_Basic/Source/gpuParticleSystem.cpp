#include "main.h"
#include "renderer.h"
#include "shaders.h"
#include "gpuParticleSystem.h"
#include "myRandom.h"

/// <summary>
/// パーティクル発生
/// </summary>
/// <param name="count">発生数</param>
void GPUParticleSystem::Emit(int count) {
	EmitGPU(count);
}

/// <summary>
/// ワンショット発生
/// </summary>
/// <param name="position">発生位置</param>
/// <param name="count">発生数</param>
void GPUParticleSystem::EmitOneShot(const Vector3& position, int count) {
	// エミッター位置を一時的に変更
	Vector3 originalPosition = m_position;
	m_position = position;
	// ワンショット発生
	EmitGPU(count > 0 ? count : m_settings.oneShotCount);
	// エミッター位置を元に戻す
	m_position = originalPosition;
}

/// <summary>
/// 初期化
/// </summary>
/// <returns>成功フラグ</returns>
bool GPUParticleSystem::Initialize() {
	// 乱数初期化
	m_randomSeed = MyRandom::GetEngine()();

	// バッファ作成
	if (!CreateBuffers()) {
		return false;
	}

	// シェーダ読み込み
	if (!LoadShaders()) {
		return false;
	}

	// ブレンドステート作成
	if (!CreateState()) {
		return false;
	}

	return true;
}

/// <summary>
/// 終了
/// </summary>
void GPUParticleSystem::Finalize() {
	// シェーダー解放
	if (m_vertexShader) {
		delete m_vertexShader;
		m_vertexShader = nullptr;
	}
	if (m_pixelShader) {
		delete m_pixelShader;
		m_pixelShader = nullptr;
	}
	if (m_updateComputeShader) {
		delete m_updateComputeShader;
		m_updateComputeShader = nullptr;
	}
	if (m_emitComputeShader) {
		delete m_emitComputeShader;
		m_emitComputeShader = nullptr;
	}
	if (m_compactComputeShader) {
		delete m_compactComputeShader;
		m_compactComputeShader = nullptr;
	}

}

/// <summary>
/// 更新
/// </summary>
/// <param name="deltaTime"></param>
void GPUParticleSystem::Update(double deltaTime) {
	float dt = static_cast<float>(deltaTime);

	if (m_isPaused) {
		return;
	}

	// パーティクル更新
	if (m_isPlaying && !m_settings.oneShot) {
		m_emitTimer += dt;
		float emitInterval = 1.0f / m_settings.emitRate;

		while (m_emitTimer >= emitInterval) {
			EmitGPU(1);
			m_emitTimer -= emitInterval;
		}
	}

	// パーティクル更新
	UpdateParticlesGPU(dt);

	// IndirectDrawモードの場合、アクティブパーティクルを収集
	if (m_drawMode == ParticleDrawMode::INDIRECT_DRAW) {
		CompactParticlesGPU();
	}
}

/// <summary>
/// 描画
/// </summary>
void GPUParticleSystem::Draw() {
	if (!m_textureSRV) {
		return;
	}

	auto context = RENDERER.GetDeviceContext();

	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// ブレンドステート設定
	float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xffffffff);

	// 頂点バッファ設定
	UINT stride = sizeof(BillboardVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// プリミティブトポロジ設定
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 描画モード分岐
	if (m_drawMode == ParticleDrawMode::CULL_DISTANCE) {
		// CullDistanceモード: 全パーティクル描画(非アクティブはシェーダー内で破棄)
		context->VSSetShaderResources(0, 1, m_particleBufferSRV.GetAddressOf());
		context->PSSetShaderResources(1, 1, &m_textureSRV);

		// インデックスドインスタンスド描画
		context->DrawIndexedInstanced(6, static_cast<UINT>(m_settings.maxParticles), 0, 0, 0);

		ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
		context->VSSetShaderResources(0, 1, nullSRV);
		context->PSSetShaderResources(1, 1, nullSRV);
	} else {
		// IndirectDrawモード: アクティブパーティクルのみ描画
		ID3D11ShaderResourceView* srvs[2] = { m_particleBufferSRV.Get(), m_activeIndicesSRV.Get() };
		context->VSSetShaderResources(0, 2, srvs);
		context->PSSetShaderResources(1, 1, &m_textureSRV);

		// 間接描画
		context->DrawIndexedInstancedIndirect(m_drawArgsBuffer.Get(), 0);

		ID3D11ShaderResourceView* nullSRV[3] = { nullptr, nullptr, nullptr };
		context->VSSetShaderResources(0, 2, nullSRV);
		context->PSSetShaderResources(1, 1, nullSRV);
	}

	// ブレンドステート解除
	RENDERER.SetATCEnable(false);
}

/// <summary>
/// パーティクル更新(GPU)
/// </summary>
/// <param name="deltaTime"></param>
void GPUParticleSystem::UpdateParticlesGPU(float deltaTime) {
	auto context = RENDERER.GetDeviceContext();

	// 更新用パラメータ設定
	UpdateParams params;
	params.deltaTime = deltaTime;
	params.gravity = m_settings.gravity;
	params.startSize = m_settings.startSize;
	params.endSize = m_settings.endSize;
	params.startColor = XMFLOAT4(m_settings.startColor.x, m_settings.startColor.y, m_settings.startColor.z, m_settings.startColor.w);
	params.endColor = XMFLOAT4(m_settings.endColor.x, m_settings.endColor.y, m_settings.endColor.z, m_settings.endColor.w);

	// パラメータバッファに転送
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_updateParamsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &params, sizeof(UpdateParams));
	context->Unmap(m_updateParamsBuffer.Get(), 0);

	// コンピュートシェーダー設定
	m_updateComputeShader->Set();

	// パラメータバッファ設定
	context->CSSetConstantBuffers(3, 1, m_updateParamsBuffer.GetAddressOf());

	// UAV設定
	ID3D11UnorderedAccessView* uavs[2] = { m_particleBufferUAV.Get(), m_freeIndicesUAV.Get() };
	context->CSSetUnorderedAccessViews(0, 2, uavs, nullptr);

	// ディスパッチ
	UINT threadGroupCount = (m_settings.maxParticles + 255) / 256;
	context->Dispatch(threadGroupCount, 1, 1);

	// リソース解除
	ID3D11UnorderedAccessView* nullUAV[2] = { nullptr, nullptr };
	context->CSSetUnorderedAccessViews(0, 2, nullUAV, nullptr);
	ID3D11Buffer* nullBuffer = nullptr;
	context->CSSetConstantBuffers(3, 1, &nullBuffer);
}

/// <summary>
/// パーティクル発生(GPU)
/// </summary>
/// <param name="count">発生数</param>
void GPUParticleSystem::EmitGPU(UINT count) {
	auto context = RENDERER.GetDeviceContext();

	// フリーリスト初期化
	if (!m_freeListInitialized) {
		// フリーインデックスUAVのカウンタを最大パーティクル数に設定
		UINT initialCount = static_cast<UINT>(m_settings.maxParticles);
		context->CSSetUnorderedAccessViews(1, 1, m_freeIndicesUAV.GetAddressOf(), &initialCount);
		m_freeListInitialized = true;
	}

	// 乱数シード更新
	m_randomSeed = (m_randomSeed * 1103515245 + 12345) & 0x7fffffff;

	// Emitパラメータ設定
	EmitParams params;
	params.emitterPosition = XMFLOAT3(m_position.x, m_position.y, m_position.z);
	params.emissionAngle = m_settings.emissionAngle;
	params.baseVelocity = XMFLOAT3(m_settings.velocity.x, m_settings.velocity.y, m_settings.velocity.z);
	params.emissionAngleVariation = m_settings.emissionAngleVariation;
	params.positionVariation = XMFLOAT3(m_settings.position.x, m_settings.position.y, m_settings.position.z);
	params.lifeTime = m_settings.lifeTime;
	params.velocityVariation = XMFLOAT3(m_settings.velocityVariation.x, m_settings.velocityVariation.y, m_settings.velocityVariation.z);
	params.startSize = m_settings.startSize;
	params.startColor = XMFLOAT4(m_settings.startColor.x, m_settings.startColor.y, m_settings.startColor.z, m_settings.startColor.w);
	params.rotationSpeed = m_settings.rotationSpeed;
	params.rotationSpeedMin = m_settings.rotationSpeedMin;
	params.rotationSpeedMax = m_settings.rotationSpeedMax;
	params.maxParticles = static_cast<UINT>(m_settings.maxParticles);
	params.emitCount = count;
	params.randomSeed = m_randomSeed;
	params.padding = XMFLOAT2(0.0f, 0.0f);

	// パラメータバッファに転送
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_emitParamsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &params, sizeof(EmitParams));
	context->Unmap(m_emitParamsBuffer.Get(), 0);

	// コンピュートシェーダー設定
	m_emitComputeShader->Set();

	// パラメータバッファ設定
	context->CSSetConstantBuffers(3, 1, m_emitParamsBuffer.GetAddressOf());

	// UAV設定
	ID3D11UnorderedAccessView* uavs[2] = { m_particleBufferUAV.Get(), m_freeIndicesUAV.Get() };
	context->CSSetUnorderedAccessViews(0, 2, uavs, nullptr);

	// ディスパッチ
	UINT threadGroupCount = (count + 255) / 256;
	context->Dispatch(threadGroupCount, 1, 1);

	// リソース解除
	ID3D11UnorderedAccessView* nullUAVs[2] = { nullptr, nullptr };
	context->CSSetUnorderedAccessViews(0, 2, nullUAVs, nullptr);
	ID3D11Buffer* nullBuffer = nullptr;
	context->CSSetConstantBuffers(3, 1, &nullBuffer);
}

/// <summary>
/// パーティクルコンパクト(GPU)
/// </summary>
void GPUParticleSystem::CompactParticlesGPU() {
	auto context = RENDERER.GetDeviceContext();

	// compact用パラメータ設定
	CompactParams params;
	params.maxParticles = static_cast<UINT>(m_settings.maxParticles);
	params.padding = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// パラメータバッファに転送
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_compactParamsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &params, sizeof(CompactParams));
	context->Unmap(m_compactParamsBuffer.Get(), 0);

	// コンピュートシェーダー設定
	m_compactComputeShader->Set();

	// パラメータバッファ設定
	context->CSSetConstantBuffers(3, 1, m_compactParamsBuffer.GetAddressOf());

	// SRV設定
	context->CSSetShaderResources(0, 1, m_particleBufferSRV.GetAddressOf());

	// UAV設定
	ID3D11UnorderedAccessView* uavs[2] = { m_activeIndicesUAV.Get(), m_drawArgsUAV.Get() };
	UINT initialCounts[2] = { 0, 0xFFFFFFFF };
	context->CSSetUnorderedAccessViews(0, 2, uavs, initialCounts);

	// ディスパッチ
	UINT threadGroupCount = (m_settings.maxParticles + 255) / 256;
	context->Dispatch(threadGroupCount, 1, 1);

	// リソース解除
	ID3D11UnorderedAccessView* nullUAVs[2] = { nullptr, nullptr };
	context->CSSetUnorderedAccessViews(0, 2, nullUAVs, nullptr);
	ID3D11ShaderResourceView* nullSRV = nullptr;
	context->CSSetShaderResources(0, 1, &nullSRV);
	ID3D11Buffer* nullBuffer = nullptr;
	context->CSSetConstantBuffers(3, 1, &nullBuffer);
}

/// <summary>
/// バッファ作成
/// </summary>
/// <returns>作成成功フラグ</returns>
bool GPUParticleSystem::CreateBuffers() {
	auto device = RENDERER.GetDevice();

	// ビルボード頂点バッファ作成
	BillboardVertex vertices[] = {
		{ XMFLOAT2(-1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) }, // 左下
		{ XMFLOAT2(-1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) }, // 左上
		{ XMFLOAT2(1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) }, // 右下
		{ XMFLOAT2(1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) }, // 右上
	};

	// 頂点バッファ作成
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(vertices);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	if (FAILED(device->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf()))) {
		return false;
	}

	// インデックスデータ
	UINT indices[] = {
		0, 1, 2,
		2, 1, 3,
	};

	// インデックスバッファ作成
	bd = {};
	bd.ByteWidth = sizeof(indices);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	initData = {};
	initData.pSysMem = indices;

	if (FAILED(device->CreateBuffer(&bd, &initData, m_indexBuffer.GetAddressOf()))) {
		return false;
	}

	// パーティクルデータバッファ作成
	bd = {};
	bd.ByteWidth = sizeof(GPUParticle) * m_settings.maxParticles;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride = sizeof(GPUParticle);

	// 初期データ(全て非アクティブ)
	GPUParticle inactiveParticle = {};
	inactiveParticle.active = 0;
	std::vector<GPUParticle> initialParticles(m_settings.maxParticles, inactiveParticle);

	D3D11_SUBRESOURCE_DATA particleData = {};
	particleData.pSysMem = initialParticles.data();
	if (FAILED(device->CreateBuffer(&bd, &particleData, m_particleBuffer.GetAddressOf()))) {
		return false;
	}

	// パーティクルデータUAV作成
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = m_settings.maxParticles;
	if (FAILED(device->CreateUnorderedAccessView(m_particleBuffer.Get(), &uavDesc, m_particleBufferUAV.GetAddressOf()))) {
		return false;
	}

	// パーティクルデータSRV作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = m_settings.maxParticles;
	if (FAILED(device->CreateShaderResourceView(m_particleBuffer.Get(), &srvDesc, m_particleBufferSRV.GetAddressOf()))) {
		return false;
	}

	// フリーインデックスバッファ作成
	bd = {};
	bd.ByteWidth = sizeof(UINT) * m_settings.maxParticles;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride = sizeof(UINT);

	// 初期データ(全インデックスをフリーリストに登録)
	std::vector<UINT> freeIndices(m_settings.maxParticles);
	for (UINT i = 0; i < m_settings.maxParticles; i++) {
		freeIndices[i] = i;
	}
	D3D11_SUBRESOURCE_DATA freeIndexData = {};
	freeIndexData.pSysMem = freeIndices.data();
	if (FAILED(device->CreateBuffer(&bd, &freeIndexData, m_freeIndicesBuffer.GetAddressOf()))) {
		return false;
	}

	// フリーインデックスUAV作成(消費用)
	uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = m_settings.maxParticles;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
	if (FAILED(device->CreateUnorderedAccessView(m_freeIndicesBuffer.Get(), &uavDesc, m_freeIndicesUAV.GetAddressOf()))) {
		return false;
	}

	// 更新パラメータバッファ作成
	bd = {};
	bd.ByteWidth = sizeof(UpdateParams);
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(device->CreateBuffer(&bd, nullptr, m_updateParamsBuffer.GetAddressOf()))) {
		return false;
	}

	// 発生パラメータバッファ作成
	bd = {};
	bd.ByteWidth = sizeof(EmitParams);
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(device->CreateBuffer(&bd, nullptr, m_emitParamsBuffer.GetAddressOf()))) {
		return false;
	}

	// IndirectDraw用リソース作成
	if (m_drawMode == ParticleDrawMode::INDIRECT_DRAW) {
		// アクティブインデックスバッファ作成
		bd = {};
		bd.ByteWidth = sizeof(UINT) * m_settings.maxParticles;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bd.StructureByteStride = sizeof(UINT);
		if (FAILED(device->CreateBuffer(&bd, nullptr, m_activeIndicesBuffer.GetAddressOf()))) {
			return false;
		}

		// アクティブインデックスUAV作成
		uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = m_settings.maxParticles;
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
		if (FAILED(device->CreateUnorderedAccessView(m_activeIndicesBuffer.Get(), &uavDesc, m_activeIndicesUAV.GetAddressOf()))) {
			return false;
		}

		// アクティブインデックスSRV作成
		srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = m_settings.maxParticles;
		if (FAILED(device->CreateShaderResourceView(m_activeIndicesBuffer.Get(), &srvDesc, m_activeIndicesSRV.GetAddressOf()))) {
			return false;
		}

		// ドローアーギュメントバッファ作成
		bd = {};
		bd.ByteWidth = sizeof(UINT) * 5;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		bd.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		HRESULT hr = device->CreateBuffer(&bd, nullptr, m_drawArgsBuffer.GetAddressOf());
		if (FAILED(hr)) {
			ErrorMessage(L"ドローアーギュメントバッファの作成に失敗しました。", hr);
			return false;
		}

		// ドローアーギュメントUAV作成
		uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_R32_UINT;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = 5;
		if (FAILED(device->CreateUnorderedAccessView(m_drawArgsBuffer.Get(), &uavDesc, m_drawArgsUAV.GetAddressOf()))) {
			return false;
		}

		// コンパクト用パラメータバッファ作成
		bd = {};
		bd.ByteWidth = sizeof(CompactParams);
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		if (FAILED(device->CreateBuffer(&bd, nullptr, m_compactParamsBuffer.GetAddressOf()))) {
			return false;
		}
	}

	return true;
}

/// <summary>
/// シェーダ読み込み
/// </summary>
/// <returns>読み込み成功</returns>
bool GPUParticleSystem::LoadShaders() {
	// 描画用シェーダー読み込み
	m_vertexShader = new VertexShader();
	if (m_drawMode == ParticleDrawMode::CULL_DISTANCE) {
		m_vertexShader->Load(L"Shader\\gpuParticleVS_CullDistance.cso");
	} else {
		m_vertexShader->Load(L"Shader\\gpuParticleVS_IndirectDraw.cso");
	}

	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\particlePS.cso");

	// 更新用コンピュートシェーダー読み込み
	m_updateComputeShader = new ComputeShader();
	m_updateComputeShader->Load(L"Shader\\particleUpdateCS.cso");

	// 発生用コンピュートシェーダー読み込み
	m_emitComputeShader = new ComputeShader();
	m_emitComputeShader->Load(L"Shader\\particleEmitCS.cso");

	// コンパクト用コンピュートシェーダー読み込み(IndirectDrawモードのみ)
	if (m_drawMode == ParticleDrawMode::INDIRECT_DRAW) {
		m_compactComputeShader = new ComputeShader();
		m_compactComputeShader->Load(L"Shader\\particleCompactCS.cso");
	}

	return true;
}

/// <summary>
/// ステート作成
/// </summary>
/// <returns>作成完了</returns>
bool GPUParticleSystem::CreateState() {
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

	if (FAILED(device->CreateBlendState(&blendDesc, m_blendState.GetAddressOf()))) {
		return false;
	}
	return true;
}
