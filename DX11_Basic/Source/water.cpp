#include "main.h"
#include "water.h"
#include "renderer.h"
#include "shaders.h"
#include "texture.h"

Water::Water()
	: m_waterSize(500.0f)
	, m_waveHeight(2.0f)
	, m_time(0.0f)
	, m_gridResolution(200)
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
void Water::Draw() const {
	auto context = RENDERER.GetDeviceContext();

	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// 定数バッファ更新
	const_cast<Water*>(this)->UpdateConstantBuffer();

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
