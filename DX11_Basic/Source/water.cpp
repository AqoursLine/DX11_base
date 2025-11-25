#include "main.h"
#include "water.h"
#include "renderer.h"
#include "shaders.h"
#include "texture.h"

Water::Water()
	: m_waterSize(500.0f)
	, m_waveHeight(2.0f)
	, m_time(0.0f)
	, m_gridResolution(800)
	, m_baseWaveFreqency1(0.02f)
	, m_baseWaveFreqency2(0.015f)
	, m_baseWaveFreqency3(0.01f)
	, m_baseWaveSpeed1(2.0f)
	, m_baseWaveSpeed2(1.5f)
	, m_baseWaveSpeed3(1.0f)
	, m_waveSharpness(2.0f)
	, m_reflectionStrength(0.6f)
	, m_refractionStrength(0.4f)
	, m_fresnelPower(3.0f)
	, m_waterClarityDepth(5.0f)
	, m_activeRippleCount(0)
	, m_indexCount(0)
{
	m_ripples.resize(MAX_RIPPLES);
}

/// <summary>
/// 波紋追加
/// </summary>
/// <param name="position">生成座標</param>
/// <param name="amplitude">波紋の振幅</param>
/// <param name="frequency">波紋の周波数</param>
/// <param name="speed">波紋の速度</param>
void Water::AddRipple(const Vector3& position, float amplitude, float frequency, float speed) {
	// 最大数チェック
	if (m_activeRippleCount >= MAX_RIPPLES) {
		// 最も古い波紋を削除
		m_activeRippleCount = MAX_RIPPLES - 1;
	}

	// 既存の波紋をシフト
	for (int i = m_activeRippleCount; i > 0; i--) {
		m_ripples[i] = m_ripples[i - 1];
	}

	// 新しい波紋を追加
	Ripple& newRipple = m_ripples[0];
	newRipple.position = position;
	newRipple.amplitude = amplitude;
	newRipple.frequency = frequency;
	newRipple.speed = speed;
	newRipple.time = 0.0f;
	newRipple.active = true;

	m_activeRippleCount++;
}

/// <summary>
/// 波高取得（CPU計算）
/// </summary>
/// <param name="position">座標</param>
/// <returns>波高</returns>
float Water::GetWaterHeight(const Vector3& position) const {
	return m_position.y + CalculateWaveHeight(position, m_time);
}

/// <summary>
/// 法線取得（CPU計算）
/// </summary>
/// <param name="position">座標</param>
/// <returns>法線</returns>
Vector3 Water::GetWaterNormal(const Vector3& position) const {
	float delta = 0.1f;

	float heightL = CalculateWaveHeight(Vector3(position.x - delta, 0.0f, position.z), m_time);
	float heightR = CalculateWaveHeight(Vector3(position.x + delta, 0.0f, position.z), m_time);
	float heightD = CalculateWaveHeight(Vector3(position.x, 0.0f, position.z - delta), m_time);
	float heightU = CalculateWaveHeight(Vector3(position.x, 0.0f, position.z + delta), m_time);

	Vector3 normal;
	normal.x = (heightL - heightR) / (2.0f * delta);
	normal.y = 1.0f;
	normal.z = (heightD - heightU) / (2.0f * delta);

	normal.Normalize();
	return normal;
}

/// <summary>
/// 初期化
/// </summary>
/// <returns>初期化成功</returns>
bool Water::Initialize() {
	auto device = RENDERER.GetDevice();

	// シェーダー読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\waterVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\waterPS.cso");

	// メッシュ生成
	CreateMesh();

	// 定数バッファ生成
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth = sizeof(WaterConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);
	if (FAILED(hr)) {
		ErrorMessage(L"水面の定数バッファの作成に失敗しました。", hr);
		return false;
	}

	// 法線マップ生成
	CreateNormalMap();

	// フォームテクスチャ生成
	CreateFoamTexture();

	// サンプラーステート生成
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	hr = device->CreateSamplerState(&samplerDesc, &m_samplerState);
	if (FAILED(hr)) {
		ErrorMessage(L"水面のサンプラーステートの作成に失敗しました。", hr);
		return false;
	}

	return true;
}

/// <summary>
/// 終了
/// </summary>
void Water::Finalize() {
	if (m_samplerState) {
		m_samplerState->Release();
		m_samplerState = nullptr;
	}

	if (m_normalMap) {
		delete m_normalMap;
		m_normalMap = nullptr;
	}
	if (m_foamTexture) {
		delete m_foamTexture;
		m_foamTexture = nullptr;
	}
	if (m_constantBuffer) {
		m_constantBuffer->Release();
		m_constantBuffer = nullptr;
	}
	if (m_indexBuffer) {
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}
	if (m_vertexBuffer) {
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
	if (m_pixelShader) {
		delete m_pixelShader;
		m_pixelShader = nullptr;
	}
	if (m_vertexShader) {
		delete m_vertexShader;
		m_vertexShader = nullptr;
	}
}

/// <summary>
/// 更新
/// </summary>
/// <param name="deltaTime">デルタタイム</param>
void Water::Update(double deltaTime) {
	float dt = static_cast<float>(deltaTime);
	m_time += dt;

	// 波紋の更新
	for (int i = 0; i < m_activeRippleCount; i++) {
		Ripple& ripple = m_ripples[i];
		ripple.time += dt;

		// 波紋の寿命チェック
		if (ripple.time > 5.0f) {
			// 波紋を無効化
			ripple.active = false;
		}
	}

	// 非アクティブな波紋をリストから削除
	int writeIndex = 0;
	for (int readIndex = 0; readIndex < m_activeRippleCount; readIndex++) {
		if (m_ripples[readIndex].active) {
			if (writeIndex != readIndex) {
				m_ripples[writeIndex] = m_ripples[readIndex];
			}
			writeIndex++;
		}
	}

	m_activeRippleCount = writeIndex;
}

/// <summary>
/// 描画
/// </summary>
void Water::Draw() {
	auto context = RENDERER.GetDeviceContext();

	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// 定数バッファ更新
	UpdateConstantBuffer();

	// テクスチャ設定
	m_normalMap->Set(0);
	m_foamTexture->Set(1);

	if (m_environmentMapSRV) {
		context->PSSetShaderResources(2, 1, &m_environmentMapSRV);
	}

	// サンプラーステート設定
	context->PSSetSamplers(0, 1, &m_samplerState);

	// ワールド行列設定
	XMMATRIX world = XMMatrixIdentity();
	world *= XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	world *= XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	world *= XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	RENDERER.SetWorldMatrix(world);

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// プリミティブトポロジ設定
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 描画
	context->DrawIndexed(m_indexCount, 0, 0);

	// サンプラーステート解除
	RENDERER.SetSamplerState();
}

/// <summary>
/// メッシュ生成
/// </summary>
void Water::CreateMesh() {
	auto device = RENDERER.GetDevice();

	int gridSize = m_gridResolution;
	float cellSize = m_waterSize / gridSize;

	// 頂点データ生成
	std::vector<VERTEX_3D> vertices;
	vertices.reserve((gridSize + 1) * (gridSize + 1));

	for (int z = 0; z <= gridSize; z++) {
		for (int x = 0; x <= gridSize; x++) {
			VERTEX_3D vertex = {};
			vertex.position = XMFLOAT4(
				(x - gridSize / 2.0f) * cellSize,
				0.0f,
				(z - gridSize / 2.0f) * cellSize,
				1.0f);
			vertex.normal = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
			vertex.texcoord = XMFLOAT4(
				static_cast<float>(x) / gridSize,
				static_cast<float>(z) / gridSize,
				0.0f,
				0.0f);
			vertex.tangent = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
			vertex.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			vertices.push_back(vertex);
		}
	}

	// 頂点バッファ生成
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = static_cast<UINT>(sizeof(VERTEX_3D) * vertices.size());
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices.data();

	HRESULT hr = device->CreateBuffer(&vbDesc, &vbData, &m_vertexBuffer);
	if (FAILED(hr)) {
		ErrorMessage(L"水面の頂点バッファの作成に失敗しました。", hr);
		return;
	}

	// インデックスデータ生成
	std::vector<UINT> indices;
	indices.reserve(gridSize * gridSize * 6);

	for (int z = 0; z < gridSize; z++) {
		for (int x = 0; x < gridSize; x++) {
			int topLeft = z * (gridSize + 1) + x;
			int topRight = topLeft + 1;
			int bottomLeft = (z + 1) * (gridSize + 1) + x;
			int bottomRight = bottomLeft + 1;
			// 三角形1
			indices.push_back(topLeft);
			indices.push_back(bottomLeft);
			indices.push_back(topRight);
			// 三角形2
			indices.push_back(topRight);
			indices.push_back(bottomLeft);
			indices.push_back(bottomRight);
		}
	}

	m_indexCount = static_cast<UINT>(indices.size());

	// インデックスバッファ生成
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices.data();

	hr = device->CreateBuffer(&ibDesc, &ibData, &m_indexBuffer);
	if (FAILED(hr)) {
		ErrorMessage(L"水面のインデックスバッファの作成に失敗しました。", hr);
		return;
	}
}

/// <summary>
/// 法線マップ生成
/// </summary>
void Water::CreateNormalMap() {
	auto device = RENDERER.GetDevice();

	m_normalMap = new Texture();

	const int size = 256;
	std::vector<BYTE> normalData(size * size * 4);

	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			float fx = static_cast<float>(x) / size;
			float fy = static_cast<float>(y) / size;

			// 2層の法線マップを生成
			float nx1 = std::sin(fx * 3.14159f * 4.0f) * 0.3f;
			float ny1 = std::sin(fy * 3.14159f * 3.0f) * 0.3f;
			float nx2 = std::sin(fx * 3.14159f * 7.0f) * 0.15f;
			float ny2 = std::sin(fy * 3.14159f * 8.0f) * 0.15f;

			Vector3 normal(nx1 + nx2, ny1 + ny2, 1.0f);
			normal.Normalize();

			int index = (y * size + x) * 4;
			normalData[index + 0] = static_cast<BYTE>((normal.x * 0.5f + 0.5f) * 255); // R
			normalData[index + 1] = static_cast<BYTE>((normal.y * 0.5f + 0.5f) * 255); // G
			normalData[index + 2] = static_cast<BYTE>((normal.z * 0.5f + 0.5f) * 255); // B
			normalData[index + 3] = 255; // A
		}
	}

	// テクスチャ生成
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = size;
	texDesc.Height = size;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = normalData.data();
	initData.SysMemPitch = size * 4;

	ID3D11Texture2D* texture = nullptr;
	HRESULT hr = device->CreateTexture2D(&texDesc, &initData, &texture);
	if (SUCCEEDED(hr)) {
		ID3D11ShaderResourceView* srv = nullptr;
		hr = device->CreateShaderResourceView(texture, nullptr, &srv);
		if (SUCCEEDED(hr)) {
			m_normalMap->SetSRV(L"WaterNormalMap", srv);
		}
		texture->Release();
	}
}

/// <summary>
/// 泡テクスチャ生成
/// </summary>
void Water::CreateFoamTexture() {
	auto device = RENDERER.GetDevice();

	m_foamTexture = new Texture();

	const int size = 256;
	std::vector<BYTE> foamData(size * size * 4);

	std::srand(12345);

	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			float noise = static_cast<float>(std::rand()) / RAND_MAX;
			BYTE value = static_cast<BYTE>(noise * 255);

			int index = (y * size + x) * 4;
			foamData[index + 0] = value;	// R
			foamData[index + 1] = value;	// G
			foamData[index + 2] = value;	// B
			foamData[index + 3] = 255;		// A
		}
	}

	// テクスチャ生成
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = size;
	texDesc.Height = size;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = foamData.data();
	initData.SysMemPitch = size * 4;

	ID3D11Texture2D* texture = nullptr;
	HRESULT hr = device->CreateTexture2D(&texDesc, &initData, &texture);
	if (SUCCEEDED(hr)) {
		ID3D11ShaderResourceView* srv = nullptr;
		hr = device->CreateShaderResourceView(texture, nullptr, &srv);
		if (SUCCEEDED(hr)) {
			m_foamTexture->SetSRV(L"WaterFoamTexture", srv);
		}
		texture->Release();
	}
}

/// <summary>
/// 定数バッファ更新
/// </summary>
void Water::UpdateConstantBuffer() {
	auto context = RENDERER.GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (SUCCEEDED(hr)) {
		WaterConstantBuffer* cb = reinterpret_cast<WaterConstantBuffer*>(mappedResource.pData);

		cb->time = m_time;
		cb->waveHeight = m_waveHeight;
		cb->waterSize = m_waterSize;
		cb->activeRippleCount = m_activeRippleCount;

		cb->baseWaveFreq1 = m_baseWaveFreqency1;
		cb->baseWaveFreq2 = m_baseWaveFreqency2;
		cb->baseWaveFreq3 = m_baseWaveFreqency3;
		cb->baseWaveSpeed1 = m_baseWaveSpeed1;
		cb->baseWaveSpeed2 = m_baseWaveSpeed2;
		cb->baseWaveSpeed3 = m_baseWaveSpeed3;
		cb->waveSharpness = m_waveSharpness;

		cb->padding3 = 0.0f;

		cb->reflectionStrength = m_reflectionStrength;
		cb->refractionStrength = m_refractionStrength;
		cb->fresnelPower = m_fresnelPower;
		cb->waterClarityDepth = m_waterClarityDepth;

		// 波紋データ設定
		// アクティブな波紋のみ設定
		for (int i = 0; i < m_activeRippleCount; i++) {
			const Ripple& ripple = m_ripples[i];
			cb->ripples[i].positionAndTime = XMFLOAT4(ripple.position.x, ripple.position.y, ripple.position.z, ripple.time);
			cb->ripples[i].params = XMFLOAT4(ripple.amplitude, ripple.frequency, ripple.speed, 1.0f);
		}

		// 非アクティブな波紋はゼロクリア
		for (int i = m_activeRippleCount; i < MAX_RIPPLES; i++) {
			cb->ripples[i].positionAndTime = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			cb->ripples[i].params = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}

		context->Unmap(m_constantBuffer, 0);
	}

	// 定数バッファをb7にセット
	context->VSSetConstantBuffers(7, 1, &m_constantBuffer);
	context->PSSetConstantBuffers(7, 1, &m_constantBuffer);
}

/// <summary>
/// 波高計算（CPU計算）
/// </summary>
/// <param name="position">座標</param>
/// <param name="time">時間</param>
/// <returns>波高</returns>
float Water::CalculateWaveHeight(const Vector3& position, float time) const {
	float height = 0.0f;
	float x = position.x;
	float z = position.z;

	// べき乗sin波で波頭を尖らせる
	auto applySharpness = [this](float wave) -> float {
		float wave01 = wave * 0.5f + 0.5f; // -1~1 -> 0~1
		wave01 = std::pow(wave01, m_waveSharpness);
		return wave01 * 2.0f - 1.0f; // 0~1 -> -1~1
	};

	float wave1 = std::sin(x * m_baseWaveFreqency1 + time * m_baseWaveSpeed1);
	float wave2 = std::sin(z * m_baseWaveFreqency2 + time * m_baseWaveSpeed2);
	float wave3 = std::sin((x + z) * m_baseWaveFreqency3 + time * m_baseWaveSpeed3);

	wave1 = applySharpness(wave1);
	wave2 = applySharpness(wave2);
	wave3 = applySharpness(wave3);

	height += wave1 * m_waveHeight * 0.3f;
	height += wave2 * m_waveHeight * 0.2f;
	height += wave3 * m_waveHeight * 0.5f;

	// 波紋の影響
	for (int i = 0; i < m_activeRippleCount; i++) {
		const Ripple& ripple = m_ripples[i];
		if (!ripple.active) continue;

		float dx = x - ripple.position.x;
		float dz = z - ripple.position.z;
		float distance = std::sqrt(dx * dx + dz * dz);

		if (distance < ripple.speed * ripple.time && ripple.time > 0.0f) {
			float wavePhase = ripple.frequency * (distance - ripple.speed * ripple.time);
			float attenuation = std::exp(-ripple.time * 0.5f); // 時間経過による減衰
			float distanceAttenuation = 1.0f / (1.0f + distance * 0.01f); // 距離による減衰

			height += std::sin(wavePhase) * ripple.amplitude * attenuation * distanceAttenuation;
		}
	}
	return height;
}
