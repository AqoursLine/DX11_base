#include "main.h"
#include "water.h"
#include "renderer.h"
#include "shaders.h"
#include "texture.h"

Water::Water()
	: m_waterSize(500.0f)
	, m_waveHeight(2.0f)
	, m_time(0.0f)
	, m_gridResolution(512)
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
	m_heightNormalData.resize(m_gridResolution * m_gridResolution);
}

/// <summary>
/// 波紋追加
/// </summary>
/// <param name="position">生成座標</param>
/// <param name="amplitude">波紋の振幅</param>
/// <param name="frequency">波紋の周波数</param>
/// <param name="speed">波紋の速度</param>
void Water::AddRipple(const Vector3& position, float amplitude, float frequency, float speed) {
	// 非アクティブな波紋を検索して追加
	for (auto& ripple : m_ripples) {
		if (!ripple.active) {
			ripple.position = position;
			ripple.amplitude = amplitude;
			ripple.frequency = frequency;
			ripple.speed = speed;
			ripple.time = 0.0f;
			ripple.active = true;
			break;
		}
	}
}

/// <summary>
/// 波高取得（CPU計算）
/// </summary>
/// <param name="position">座標</param>
/// <returns>波高</returns>
float Water::GetWaterHeight(const Vector3& position) const {
	// ローカル座標に変換
	float localX = position.x - m_position.x + (m_waterSize * 0.5f);
	float localZ = position.z - m_position.z + (m_waterSize * 0.5f);

	// グリッド座標に変換
	float gridX = (localX / m_waterSize) * (m_gridResolution - 1);
	float gridZ = (localZ / m_waterSize) * (m_gridResolution - 1);

	// 範囲外チェック
	if (gridX < 0.0f || gridX >= m_gridResolution - 1 ||
		gridZ < 0.0f || gridZ >= m_gridResolution - 1) {
		return m_position.y;
	}

	// 周囲4点のインデックス取得
	int x0 = static_cast<int>(gridX);
	int z0 = static_cast<int>(gridZ);
	int x1 = x0 + 1;
	int z1 = z0 + 1;

	// 補間係数
	float tx = gridX - x0;
	float tz = gridZ - z0;

	// 周囲4点の波高取得
	float h00 = m_heightNormalData[z0 * m_gridResolution + x0].height;
	float h10 = m_heightNormalData[z0 * m_gridResolution + x1].height;
	float h01 = m_heightNormalData[z1 * m_gridResolution + x0].height;
	float h11 = m_heightNormalData[z1 * m_gridResolution + x1].height;

	// バイリニア補間
	float height = BilinearInterpolate(h00, h10, h01, h11, tx, tz);

	return m_position.y + height * m_waveHeight;
}

/// <summary>
/// 法線取得（CPU計算）
/// </summary>
/// <param name="position">座標</param>
/// <returns>法線</returns>
Vector3 Water::GetWaterNormal(const Vector3& position) const {
	// ローカル座標に変換
	float localX = position.x - m_position.x + (m_waterSize * 0.5f);
	float localZ = position.z - m_position.z + (m_waterSize * 0.5f);

	// グリッド座標に変換
	float gridX = (localX / m_waterSize) * (m_gridResolution - 1);
	float gridZ = (localZ / m_waterSize) * (m_gridResolution - 1);

	// 範囲外チェック
	if (gridX < 0.0f || gridX >= m_gridResolution - 1 ||
		gridZ < 0.0f || gridZ >= m_gridResolution - 1) {
		return Vector3::UP;
	}

	// 周囲4点のインデックス取得
	int x0 = static_cast<int>(gridX);
	int z0 = static_cast<int>(gridZ);
	int x1 = x0 + 1;
	int z1 = z0 + 1;

	// 補間係数
	float tx = gridX - x0;
	float tz = gridZ - z0;

	// 周囲4点の法線取得
	auto& n00 = m_heightNormalData[z0 * m_gridResolution + x0];
	auto& n10 = m_heightNormalData[z0 * m_gridResolution + x1];
	auto& n01 = m_heightNormalData[z1 * m_gridResolution + x0];
	auto& n11 = m_heightNormalData[z1 * m_gridResolution + x1];

	// バイリニア補間
	Vector3 normal = BilinearInterpolateVector3(n00.normal, n10.normal, n01.normal, n11.normal, tx, tz);

	normal.Normalize();
	return normal;
}

/// <summary>
/// 初期化
/// </summary>
/// <returns>初期化成功</returns>
bool Water::Initialize() {
	// メッシュ生成
	CreateMesh();

	// 法線マップ生成
	CreateNormalMap();

	// フォームテクスチャ生成
	CreateFoamTexture();

	// シェーダー読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\waterVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\waterPS.cso");
	m_computeShader = new ComputeShader();
	m_computeShader->Load(L"Shader\\waterCS.cso");

	// ComputeShader関連リソース生成
	CreateComputeResources();

	// 定数バッファ生成
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth = sizeof(WaterConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;

	HRESULT hr = RENDERER.GetDevice()->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);
	if (FAILED(hr)) {
		ErrorMessage(L"水面の定数バッファの作成に失敗しました。", hr);
		return false;
	}

	// サンプラーステート生成
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	hr = RENDERER.GetDevice()->CreateSamplerState(&samplerDesc, &m_samplerState);
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
	if (m_normalMap) {
		delete m_normalMap;
		m_normalMap = nullptr;
	}
	if (m_foamTexture) {
		delete m_foamTexture;
		m_foamTexture = nullptr;
	}
	if (m_pixelShader) {
		delete m_pixelShader;
		m_pixelShader = nullptr;
	}
	if (m_vertexShader) {
		delete m_vertexShader;
		m_vertexShader = nullptr;
	}
	if (m_computeShader) {
		delete m_computeShader;
		m_computeShader = nullptr;
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
	m_activeRippleCount = 0;
	for (auto& ripple : m_ripples) {
		if (ripple.active) {
			ripple.time += dt;

			// 一定時間経過で非アクティブ化
			if (ripple.time > 5.0f) {
				ripple.active = false;
			} else {
				m_activeRippleCount++;
			}
		}
	}

	// ComputeShader実行
	DispatchComputeShader();

	// 高さと法線データコピー
	CopyHeightNormalData();
}

/// <summary>
/// 描画
/// </summary>
void Water::Draw() {
	auto context = RENDERER.GetDeviceContext();

	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// ワールド行列設定
	XMMATRIX world = XMMatrixIdentity();
	world *= XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	world *= XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	world *= XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	RENDERER.SetWorldMatrix(world);

	// 水面パラメータを定数バッファに転送
	SHADER_PROPERTIES props = {};
	props.params1 = Vector4(m_waterSize, m_reflectionStrength, m_refractionStrength, m_fresnelPower);
	props.params2 = Vector4(m_waterClarityDepth, 0.0f, 0.0f, 0.0f);
	RENDERER.SetShaderProperties(props);

	// 高さ・法線テクスチャ設定
	context->VSSetShaderResources(3, 1, &m_heightNormalSRV);

	// 法線マップ設定
	if (m_normalMap) {
		auto srv = m_normalMap->GetSRV();
		context->PSSetShaderResources(0, 1, &srv);
	}

	// フォームテクスチャ設定
	if (m_foamTexture) {
		auto srv = m_foamTexture->GetSRV();
		context->PSSetShaderResources(1, 1, &srv);
	}

	// 環境マップ設定
	if (m_environmentMapSRV) {
		context->PSSetShaderResources(2, 1, &m_environmentMapSRV);
	}


	// サンプラーステート設定
	context->VSSetSamplers(0, 1, &m_samplerState);
	context->PSSetSamplers(0, 1, &m_samplerState);


	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

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
	float halfSize = m_waterSize * 0.5f;

	// 頂点データ生成
	std::vector<VERTEX_3D> vertices;
	vertices.reserve((gridSize + 1) * (gridSize + 1));

	for (int z = 0; z <= gridSize; z++) {
		for (int x = 0; x <= gridSize; x++) {
			VERTEX_3D vertex = {};
			vertex.position = XMFLOAT4(
				x * cellSize - halfSize,
				0.0f,
				z * cellSize - halfSize,
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
/// コンピュートシェーダー用リソースの生成
/// </summary>
void Water::CreateComputeResources() {
	auto device = RENDERER.GetDevice();

	// 高さと法線を格納するテクスチャ生成
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = m_gridResolution;
	texDesc.Height = m_gridResolution;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // xyz:法線、w:高さ
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

	device->CreateTexture2D(&texDesc, nullptr, &m_heightNormalTexture);

	// UAV生成
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = texDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	device->CreateUnorderedAccessView(m_heightNormalTexture.Get(), &uavDesc, &m_heightNormalUAV);

	// SRV生成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(m_heightNormalTexture.Get(), &srvDesc, &m_heightNormalSRV);

	// CPU読み取り用バッファ生成
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.BindFlags = 0;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	device->CreateTexture2D(&texDesc, nullptr, &m_stagingTexture);

}

/// <summary>
/// コンピュートシェーダーのディスパッチ
/// </summary>
void Water::DispatchComputeShader() {
	auto context = RENDERER.GetDeviceContext();

	// 定数バッファ更新
	WaterConstantBuffer cbData = {};
	cbData.time = m_time;
	cbData.waveHeight = m_waveHeight;
	cbData.waterSize = m_waterSize;
	cbData.activeRippleCount = m_activeRippleCount;

	// 基本波パラメータ
	cbData.baseWaveFreq1 = m_baseWaveFreqency1;
	cbData.baseWaveFreq2 = m_baseWaveFreqency2;
	cbData.baseWaveFreq3 = m_baseWaveFreqency3;
	cbData.baseWaveSpeed1 = m_baseWaveSpeed1;
	cbData.baseWaveSpeed2 = m_baseWaveSpeed2;
	cbData.baseWaveSpeed3 = m_baseWaveSpeed3;
	cbData.waveSharpness = m_waveSharpness;
	cbData.gridResolution = m_gridResolution;

	// 波紋データ
	int index = 0;
	for (const auto& ripple : m_ripples) {
		if (ripple.active && index < MAX_RIPPLES) {
			cbData.ripples[index].positionAndTime = XMFLOAT4(ripple.position.x, ripple.position.z, 0.0f, ripple.time);
			cbData.ripples[index].params = XMFLOAT4(ripple.amplitude, ripple.frequency, ripple.speed, 1.0f);
			index++;
		}
	}

	context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cbData, 0, 0);

	// シェーダー設定
	m_computeShader->Set();
	// 定数バッファ設定
	context->CSSetConstantBuffers(7, 1, &m_constantBuffer);
	// UAV設定
	context->CSSetUnorderedAccessViews(0, 1, &m_heightNormalUAV, nullptr);

	// ディスパッチ
	UINT threadGroupX = (m_gridResolution + 15) / 16;
	UINT threadGroupY = (m_gridResolution + 15) / 16;
	context->Dispatch(threadGroupX, threadGroupY, 1);

	// UAV解除
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

}

/// <summary>
/// 高さと法線データをコピー
/// </summary>
void Water::CopyHeightNormalData() {
	auto context = RENDERER.GetDeviceContext();

	// ステージングテクスチャにコピー
	context->CopyResource(m_stagingTexture.Get(), m_heightNormalTexture.Get());

	// マップ取得
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = context->Map(m_stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
	if (SUCCEEDED(hr)) {
		float* dataPtr = reinterpret_cast<float*>(mappedResource.pData);

		for (int y = 0; y < m_gridResolution; y++) {
			for (int x = 0; x < m_gridResolution; x++) {
				int srcIndex = (y * mappedResource.RowPitch / sizeof(float)) + (x * 4);
				int dstIndex = y * m_gridResolution + x;

				m_heightNormalData[dstIndex].normal = Vector3(
					dataPtr[srcIndex + 0],
					dataPtr[srcIndex + 1],
					dataPtr[srcIndex + 2]);
				m_heightNormalData[dstIndex].height = dataPtr[srcIndex + 3];
			}
		}

		context->Unmap(m_stagingTexture.Get(), 0);
	}
}

/// <summary>
/// バイリニア補間(スカラ―値)
/// </summary>
/// <param name="p11">左上</param>
/// <param name="p12">右上</param>
/// <param name="p21">左下</param>
/// <param name="p22">右下</param>
/// <param name="tx">X方向の補間係数</param>
/// <param name="ty">Y方向の補間係数</param>
/// <returns></returns>
float Water::BilinearInterpolate(float p11, float p12, float p21, float p22, float tx, float ty) const {
	float r1 = p11 * (1.0f - tx) + p12 * tx;
	float r2 = p21 * (1.0f - tx) + p22 * tx;
	return r1 * (1.0f - ty) + r2 * ty;
}

/// <summary>
/// バイリニア補間(ベクトル値)
/// </summary>
/// <param name="p11">左上</param>
/// <param name="p12">右上</param>
/// <param name="p21">左下</param>
/// <param name="p22">右下</param>
/// <param name="tx">X方向の補間係数</param>
/// <param name="ty">Y方向の補間係数</param>
/// <returns></returns>
Vector3 Water::BilinearInterpolateVector3(const Vector3& p11, const Vector3& p12, const Vector3& p21, const Vector3& p22, float tx, float ty) const {
	Vector3 r1 = p11 * (1.0f - tx) + p12 * tx;
	Vector3 r2 = p21 * (1.0f - tx) + p22 * tx;
	return r1 * (1.0f - ty) + r2 * ty;
}

