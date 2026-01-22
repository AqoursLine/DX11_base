#include "water.h"

#include "renderer.h"
#include "shaders.h"
#include "texture.h"

/// <summary>
/// コンストラクタ
/// </summary>
Water::Water()
	: m_waterSize(WaterConstants::DEFAULT_WATER_SIZE)
	, m_waveHeight(WaterConstants::DEFAULT_WAVE_HEIGHT)
	, m_time(0.0f)
	, m_gridResolution(WaterConstants::GRID_RESOLUTION)
	, m_activeRippleCount(0)
	, m_vertexCount(0)
	, m_indexCount(0)
	, m_needsInitialCopy(true)
{
}

/// <summary>
/// 波紋追加
/// </summary>
/// <param name="position">座標</param>
/// <param name="amplitude">振幅</param>
/// <param name="frequency">周波数</param>
/// <param name="speed">速度</param>
void Water::AddRipple(const Vector3& position, float amplitude, float frequency, float speed) {
	// 最大数チェック
	if (m_activeRippleCount >= WaterConstants::MAX_RIPPLES) {
		// 最も古い波紋を上書き
		m_activeRippleCount = WaterConstants::MAX_RIPPLES - 1;
	}

	// 新しい波紋を先頭に挿入(既存を後ろにシフト)
	for (int  i = m_activeRippleCount; i > 0; i--) {
		m_ripples[i] = m_ripples[i - 1];
	}

	// 新しい波紋設定
	m_ripples[0].position = position;
	m_ripples[0].amplitude = amplitude;
	m_ripples[0].frequency = frequency;
	m_ripples[0].speed = speed;
	m_ripples[0].time = 0.0f;
	m_ripples[0].active = true;

	m_activeRippleCount++;
}

/// <summary>
/// 波紋クリア
/// </summary>
void Water::ClearRipples() {
	for (auto& ripple : m_ripples) {
		ripple.active = false;
	}
	m_activeRippleCount = 0;
}

/// <summary>
/// 波高取得（CPU計算）
/// </summary>
/// <param name="position">座標</param>
/// <returns>高さ</returns>
float Water::GetWaterHeight(const Vector3& position) const {
	return m_position.y + CalculateWaveHeight(position, m_time);
}

/// <summary>
/// 法線取得（CPU計算）
/// </summary>
/// <param name="position">座標</param>
/// <returns>法線ベクトル</returns>
Vector3 Water::GetWaterNormal(const Vector3& position) const {
	constexpr float delta = 0.1f;

	const float heightL = CalculateWaveHeight(Vector3(position.x - delta, 0.0f, position.z), m_time);
	const float heightR = CalculateWaveHeight(Vector3(position.x + delta, 0.0f, position.z), m_time);
	const float heightD = CalculateWaveHeight(Vector3(position.x, 0.0f, position.z - delta), m_time);
	const float heightU = CalculateWaveHeight(Vector3(position.x, 0.0f, position.z + delta), m_time);

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
	// メッシュ生成
	if (!CreateMesh()) {
		return false;
	}

	// コンピュートシェーダー初期化
	if (!InitializeComputeShader()) {
		Finalize();
		return false;
	}

	// レンダーリソース初期化
	if (!InitializeRenderResources()) {
		Finalize();
		return false;
	}

	// テクスチャ生成
	if (!CreateTextures()) {
		Finalize();
		return false;
	}

	m_needsInitialCopy = true;

	return true;
}

/// <summary>
/// 終了
/// </summary>
void Water::Finalize() {
	// シェーダー解放
	if (m_computeShader) {
		delete m_computeShader;
		m_computeShader = nullptr;
	}

	if (m_vertexShader) {
		delete m_vertexShader;
		m_vertexShader = nullptr;
	}

	if (m_pixelShader) {
		delete m_pixelShader;
		m_pixelShader = nullptr;
	}

	// テクスチャ解放
	if (m_normalMap) {
		delete m_normalMap;
		m_normalMap = nullptr;
	}
	if (m_foamTexture) {
		delete m_foamTexture;
		m_foamTexture = nullptr;
	}
}

/// <summary>
/// 更新
/// </summary>
/// <param name="deltaTime">デルタタイム</param>
void Water::Update(double deltaTime) {
	const float dt = static_cast<float>(deltaTime);
	m_time += dt;

	// 頂点情報をコピー
	if (m_needsInitialCopy) {
		auto context = RENDERER.GetDeviceContext();
		context->CopyResource(m_computeBuffer.Get(), m_vertexBuffer.Get());
		m_needsInitialCopy = false;
	}

	// 波紋更新
	UpdateRipples(dt);

	// コンピュートシェーダーで波更新
	UpdateWaveWithComputeShader();
}

/// <summary>
/// 描画
/// </summary>
void Water::Draw() {
	auto context = RENDERER.GetDeviceContext();

	// シェーダーセット
	m_vertexShader->Set();
	m_pixelShader->Set();

	// 描画用定数バッファ更新
	UpdateRenderConstants();

	// テクスチャセット
	m_normalMap->Set(0);
	m_foamTexture->Set(1);
	if (m_environmentMapSRV) {
		context->PSSetShaderResources(2, 1, &m_environmentMapSRV);
	}

	// サンプラーステートセット
	context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

	// ワールド行列設定
	XMMATRIX worldMatrix = XMMatrixIdentity();
	worldMatrix *= XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	worldMatrix *= XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	worldMatrix *= XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	RENDERER.SetWorldMatrix(worldMatrix);

	// 頂点バッファセット
	const UINT stride = sizeof(VERTEX_3D);
	const UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 描画
	context->DrawIndexed(m_indexCount, 0, 0);

	// サンプラーステート解除
	RENDERER.SetSamplerState();
}

/// <summary>
/// メッシュ生成
/// </summary>
/// <returns>生成成功</returns>
bool Water::CreateMesh() {
	auto device = RENDERER.GetDevice();

	const int gridSize = m_gridResolution;
	const float cellSize = m_waterSize / static_cast<float>(gridSize);
	const int vertexCountPerRow = gridSize + 1;

	// 頂点数計算
	m_vertexCount = vertexCountPerRow * vertexCountPerRow;

	// 頂点データ生成
	std::vector<VERTEX_3D> vertices;
	vertices.reserve(m_vertexCount);

	for (int z = 0; z < vertexCountPerRow; z++) {
		for (int x = 0; x < vertexCountPerRow; x++) {
			VERTEX_3D vertex = {};

			// 位置
			vertex.position.x = (x - gridSize * 0.5f) * cellSize;
			vertex.position.y = 0.0f;
			vertex.position.z = (z - gridSize * 0.5f) * cellSize;
			vertex.position.w = 1.0f;

			// 法線
			vertex.normal = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

			// テクスチャ座標
			vertex.texcoord.x = static_cast<float>(x) / static_cast<float>(gridSize);
			vertex.texcoord.y = static_cast<float>(z) / static_cast<float>(gridSize);
			vertex.texcoord.z = 0.0f;
			vertex.texcoord.w = 0.0f;

			// 接線
			vertex.tangent = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);

			// ディフューズカラー
			vertex.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

			vertices.push_back(vertex);
		}
	}

	// 頂点バッファ作成
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = static_cast<UINT>(sizeof(VERTEX_3D) * vertices.size());
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();

	HRESULT hr = device->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"水面頂点バッファの作成に失敗しました。", hr);
		return false;
	}

	// インデックスデータ生成
	std::vector<UINT> indices;
	const int triangleCount = gridSize * gridSize * 2;
	indices.reserve(triangleCount * 3);

	for (int z = 0; z < gridSize; z++) {
		for (int x = 0; x < gridSize; x++) {
			const int topLeft = z * vertexCountPerRow + x;
			const int topRight = topLeft + 1;
			const int bottomLeft = (z + 1) * vertexCountPerRow + x;
			const int bottomRight = bottomLeft + 1;

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

	// インデックスバッファ作成
	bd = {};
	bd.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	initData = {};
	initData.pSysMem = indices.data();

	hr = device->CreateBuffer(&bd, &initData, m_indexBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"水面インデックスバッファの作成に失敗しました。", hr);
		return false;
	}


	return true;
}

/// <summary>
/// コンピュートシェーダー初期化
/// </summary>
/// <returns>初期化成功</returns>
bool Water::InitializeComputeShader() {
	auto device = RENDERER.GetDevice();

	// コンピュートシェーダー読み込み
	m_computeShader = new ComputeShader();
	m_computeShader->Load(L"Shader\\waterUpdateCS.cso");

	// コンピュート定数バッファ作成
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(WaveComputeConstants);
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = device->CreateBuffer(&bd, nullptr, m_computeConstantBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"水面コンピュート定数バッファの作成に失敗しました。", hr);
		return false;
	}

	// 頂点計算結果格納用バッファ作成
	bd = {};
	bd.ByteWidth = static_cast<UINT>(sizeof(VERTEX_3D) * m_vertexCount);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride = sizeof(VERTEX_3D);
	hr = device->CreateBuffer(&bd, nullptr, m_computeBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"水面コンピュートバッファの作成に失敗しました。", hr);
		return false;
	}

	// UAV作成
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = m_vertexCount;

	hr = device->CreateUnorderedAccessView(m_computeBuffer.Get(), &uavDesc, m_heightUAV.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"水面UAVの作成に失敗しました。", hr);
		return false;
	}

	return true;
}

/// <summary>
/// レンダーリソース初期化
/// </summary>
/// <returns>初期化成功</returns>
bool Water::InitializeRenderResources() {
	auto device = RENDERER.GetDevice();

	// シェーダー読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\waterVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\waterPS.cso");

	// レンダー定数バッファ作成
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(WaterRenderConstants);
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = device->CreateBuffer(&bd, nullptr, m_renderConstantBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"水面レンダー定数バッファの作成に失敗しました。", hr);
		return false;
	}

	// サンプラーステート
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	hr = device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"水面サンプラーステートの作成に失敗しました。", hr);
		return false;
	}

	return true;
}

/// <summary>
/// テクスチャ生成
/// </summary>
/// <returns>初期化成功</returns>
bool Water::CreateTextures() {
	auto device = RENDERER.GetDevice();

	constexpr int TEXTURE_SIZE = 256;

	// 法線マップ生成
	m_normalMap = new Texture();
	std::vector<BYTE> normalData(TEXTURE_SIZE * TEXTURE_SIZE * 4);

	for (int y = 0; y < TEXTURE_SIZE; y++) {
		for (int x = 0; x < TEXTURE_SIZE; x++) {
			const float fx = (static_cast<float>(x) / static_cast<float>(TEXTURE_SIZE));
			const float fy = (static_cast<float>(y) / static_cast<float>(TEXTURE_SIZE));

			// 2層の法線マップを生成
			const float nx1 = std::sin(fx * XM_PI * 4.0f) * 0.3f;
			const float ny1 = std::sin(fy * XM_PI * 3.0f) * 0.3f;
			const float nx2 = std::sin(fx * XM_PI * 7.0f) * 0.15f;
			const float ny2 = std::sin(fy * XM_PI * 8.0f) * 0.15f;

			Vector3 normal = Vector3(nx1 + nx2, ny1 + ny2, 1.0f);
			normal.Normalize();

			const int index = (y * TEXTURE_SIZE + x) * 4;
			normalData[index + 0] = static_cast<BYTE>((normal.x * 0.5f + 0.5f) * 255.0f); // R
			normalData[index + 1] = static_cast<BYTE>((normal.y * 0.5f + 0.5f) * 255.0f); // G
			normalData[index + 2] = static_cast<BYTE>((normal.z * 0.5f + 0.5f) * 255.0f); // B
			normalData[index + 3] = 255; // A

		}
	}

	// 法線マップテクスチャ作成
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = TEXTURE_SIZE;
	texDesc.Height = TEXTURE_SIZE;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = normalData.data();
	initData.SysMemPitch = TEXTURE_SIZE * 4;

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

	// 泡テクスチャ作成
	m_foamTexture = new Texture();
	std::vector<BYTE> foamData(TEXTURE_SIZE * TEXTURE_SIZE * 4);

	std::srand(12345); // 固定シードで同じパターンにする

	for (int i = 0; i < TEXTURE_SIZE * TEXTURE_SIZE; i++) {
		const float noise = static_cast<float>(std::rand()) / RAND_MAX;
		const BYTE value = static_cast<BYTE>(noise * 255);

		const int index = i * 4;
		foamData[index + 0] = value; // R
		foamData[index + 1] = value; // G
		foamData[index + 2] = value; // B
		foamData[index + 3] = 255; // A
	}

	// 泡テクスチャ作成
	initData.pSysMem = foamData.data();
	hr = device->CreateTexture2D(&texDesc, &initData, &texture);
	if (SUCCEEDED(hr)) {
		ID3D11ShaderResourceView* srv = nullptr;
		hr = device->CreateShaderResourceView(texture, nullptr, &srv);
		if (SUCCEEDED(hr)) {
			m_foamTexture->SetSRV(L"WaterFoamTexture", srv);
		}
		texture->Release();
	}


	return true;
}

/// <summary>
/// コンピュートシェーダーで波更新
/// </summary>
void Water::UpdateWaveWithComputeShader() {
	auto context = RENDERER.GetDeviceContext();

	// 定数バッファ更新
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	HRESULT hr = context->Map(m_computeConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (SUCCEEDED(hr)) {
		WaveComputeConstants* constants = static_cast<WaveComputeConstants*>(mappedResource.pData);
		UpdateComputeConstants(*constants);
		context->Unmap(m_computeConstantBuffer.Get(), 0);
	}

	// コンピュートシェーダー設定
	m_computeShader->Set();
	context->CSSetConstantBuffers(0, 1, m_computeConstantBuffer.GetAddressOf());
	context->CSSetUnorderedAccessViews(0, 1, m_heightUAV.GetAddressOf(), nullptr);

	// ディスパッチ
	const int dispatchX = WaterConstants::DISPATCH_GROUPS_X;
	const int dispatchY = WaterConstants::DISPATCH_GROUPS_Y;
	context->Dispatch(dispatchX, dispatchY, 1);

	// アンセット
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

	// 頂点バッファをコンピュート結果に差し替え
	ComPtr<ID3D11Resource> uavResource;
	m_heightUAV->GetResource(&uavResource);

	context->CopyResource(m_vertexBuffer.Get(), uavResource.Get());
}

/// <summary>
/// コンピュート定数バッファ更新
/// </summary>
/// <param name="constants">コンピュートシェーダー用定数構造体</param>
void Water::UpdateComputeConstants(WaveComputeConstants& constants) const {
	// 基本パラメータ
	constants.time = m_time;
	constants.waveHeight = m_waveHeight;
	constants.waterSize = m_waterSize;
	constants.activeRippleCount = m_activeRippleCount;

	// 基本波パラメータ
	constants.baseWaveFreq1 = m_waveParams.frequency1;
	constants.baseWaveFreq2 = m_waveParams.frequency2;
	constants.baseWaveFreq3 = m_waveParams.frequency3;
	constants.baseWaveSpeed1 = m_waveParams.speed1;
	constants.baseWaveSpeed2 = m_waveParams.speed2;
	constants.baseWaveSpeed3 = m_waveParams.speed3;
	constants.waveSharpness = m_waveParams.sharpness;

	// グリッド情報
	constants.gridResolution = m_gridResolution;
	constants.cellSize = m_waterSize / m_gridResolution;
	constants.normalDelta = 0.1f;
	constants.padding1 = 0.0f;
	constants.padding2 = 0.0f;

	// 波紋データをコピー
	for (int i = 0; i < WaterConstants::MAX_RIPPLES; i++) {
		if (i < m_activeRippleCount && m_ripples[i].active) {
			const Ripple& ripple = m_ripples[i];
			constants.ripples[i].positionAndTime = XMFLOAT4(ripple.position.x, ripple.position.y, ripple.position.z, ripple.time);
			constants.ripples[i].params = XMFLOAT4(ripple.amplitude, ripple.frequency, ripple.speed, 1.0f);
		} else {
			constants.ripples[i].positionAndTime = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			constants.ripples[i].params = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
}

/// <summary>
/// レンダー定数バッファ更新
/// </summary>
void Water::UpdateRenderConstants() {
	auto context = RENDERER.GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	HRESULT hr = context->Map(m_renderConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (SUCCEEDED(hr)) {
		WaterRenderConstants* constants = static_cast<WaterRenderConstants*>(mappedResource.pData);

		// タイミング情報
		constants->time = m_time;
		constants->waveHeight = m_waveHeight;
		constants->waterSize = m_waterSize;
		constants->activeRippleCount = m_activeRippleCount;

		// 基本波パラメータ
		constants->baseWaveFreq1 = m_waveParams.frequency1;
		constants->baseWaveFreq2 = m_waveParams.frequency2;
		constants->baseWaveFreq3 = m_waveParams.frequency3;
		constants->baseWaveSpeed1 = m_waveParams.speed1;
		constants->baseWaveSpeed2 = m_waveParams.speed2;
		constants->baseWaveSpeed3 = m_waveParams.speed3;
		constants->waveSharpness = m_waveParams.sharpness;
		constants->padding1 = 0;

		// 環境マッピング用パラメータ
		constants->reflectionStrength = m_envParams.reflectionStrength;
		constants->refractionStrength = m_envParams.refractionStrength;
		constants->fresnelPower = m_envParams.fresnelPower;
		constants->waterClarityDepth = m_envParams.waterClarityDepth;

		// 波紋データをコピー
		for (int i = 0; i < WaterConstants::MAX_RIPPLES; i++) {
			if (i < m_activeRippleCount && m_ripples[i].active) {
				const Ripple& ripple = m_ripples[i];
				constants->ripples[i].positionAndTime = XMFLOAT4(ripple.position.x, ripple.position.y, ripple.position.z, ripple.time);
				constants->ripples[i].params = XMFLOAT4(ripple.amplitude, ripple.frequency, ripple.speed, 1.0f);
			} else {
				constants->ripples[i].positionAndTime = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
				constants->ripples[i].params = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			}
		}

		context->Unmap(m_renderConstantBuffer.Get(), 0);
	}

	// 定数バッファセット
	context->VSSetConstantBuffers(7, 1, m_renderConstantBuffer.GetAddressOf());
	context->PSSetConstantBuffers(7, 1, m_renderConstantBuffer.GetAddressOf());
}

/// <summary>
/// 波紋管理更新
/// </summary>
/// <param name="deltaTime">デルタタイム</param>
void Water::UpdateRipples(float deltaTime) {
	// 波紋の寿命更新
	for (int i = 0; i < m_activeRippleCount; i++) {
		m_ripples[i].time += deltaTime;

		// 寿命チェック
		if (m_ripples[i].time > WaterConstants::RIPPLE_LIFETIME) {
			m_ripples[i].active = false;
		}
	}

	// 非アクティブな波紋をリストから削除
	RemoveInactiveRipples();
}

/// <summary>
/// 非アクティブな波紋削除
/// </summary>
void Water::RemoveInactiveRipples() {
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
/// 波高計算（CPU計算）
/// </summary>
/// <param name="position">座標</param>
/// <param name="time">時間</param>
/// <returns>高さ</returns>
float Water::CalculateWaveHeight(const Vector3& position, float time) const {
	return CalculateBaseWaves(position.x, position.z, time) +
		CalculateRippleWaves(position.x, position.z, time);
}

/// <summary>
/// 基本波計算（CPU計算）
/// </summary>
/// <param name="x">x座標</param>
/// <param name="z">z座標</param>
/// <param name="time">時間</param>
/// <returns>高さ</returns>
float Water::CalculateBaseWaves(float x, float z, float time) const {
	// べき乗sin波で波頭を尖らせる
	auto applySharpness = [this](float wave) -> float {
		const float wave01 = wave * 0.5f + 0.5f; // -1～1 -> 0～1
		const float powered = std::pow(wave01, m_waveParams.sharpness);
		return powered * 2.0f - 1.0f; // 0～1 -> -1～1
		};

	// 3つの基本波
	float wave1 = std::sin(x * m_waveParams.frequency1 + time * m_waveParams.speed1);
	float wave2 = std::sin(z * m_waveParams.frequency2 + time * m_waveParams.speed2);
	float wave3 = std::sin((x + z) * m_waveParams.frequency3 + time * m_waveParams.speed3);

	wave1 = applySharpness(wave1);
	wave2 = applySharpness(wave2);
	wave3 = applySharpness(wave3);

	return (wave1 * 0.3f + wave2 * 0.2f + wave3 * 0.5f) * m_waveHeight;
}

/// <summary>
/// 波紋波計算（CPU計算）
/// </summary>
/// <param name="x">x座標</param>
/// <param name="z">z座標</param>
/// <param name="time">時間</param>
/// <returns>高さ</returns>
float Water::CalculateRippleWaves(float x, float z, float time) const {
	float height = 0.0f;

	for (int i = 0; i < m_activeRippleCount; i++) {
		const Ripple& ripple = m_ripples[i];
		if (!ripple.active) {
			continue;
		}

		const float dx = x - ripple.position.x;
		const float dz = z - ripple.position.z;
		const float distance = std::sqrt(dx * dx + dz * dz);

		if (distance < ripple.speed * ripple.time && ripple.time > 0.0f) {
			const float wavePhase = ripple.frequency * (distance - ripple.speed * ripple.time);
			const float timeAttenuation = std::exp(-ripple.time * WaterConstants::RIPPLE_TIME_ATTENUATION);
			const float distanceAttenuation = 1.0f / (1.0f + distance * WaterConstants::RIPPLE_DISTANCE_ATTENUATION);

			height += std::sin(wavePhase) * ripple.amplitude * timeAttenuation * distanceAttenuation;
		}
	}

	return height;
}
