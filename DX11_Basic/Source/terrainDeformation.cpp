#include "terrainDeformation.h"
#include "DX11/renderer.h"

TerrainDeformation::TerrainDeformation() :
	m_heightTexture(nullptr),
	m_heightSRV(nullptr),
	m_heightUAV(nullptr),
	m_originalHeightTexture(nullptr),
	m_decalTexture(nullptr),
	m_decalSRV(nullptr),
	m_decalUAV(nullptr),
	m_deformationCS(nullptr),
	m_decalCS(nullptr),
	m_hoofPrintBuffer(nullptr),
	m_hoofPrintSRV(nullptr),
	m_deformationParamsBuffer(nullptr),
	m_terrainWidth(1000.0f),
	m_terrainHeight(1000.0f),
	m_heightMapWidth(1024),
	m_heightMapHeight(1024),
	m_maxDeformationDepth(0.5f),
	m_deformationRadius(2.0f),
	m_recoverySpeed(0.1f) {
}

TerrainDeformation::~TerrainDeformation() {
	Finalize();
}

bool TerrainDeformation::Initialize(float terrainWidth, float terrainHeight, int heightMapWidth, int heightMapHeight) {
	m_terrainWidth = terrainWidth;
	m_terrainHeight = terrainHeight;
	m_heightMapWidth = heightMapWidth;
	m_heightMapHeight = heightMapHeight;

	if (!CreateHeightTextures()) return false;
	if (!CreateDecalTextures()) return false;
	if (!CreateComputeShaders()) return false;
	if (!CreateBuffers()) return false;

	return true;
}

void TerrainDeformation::Finalize() {
	if (m_heightTexture) m_heightTexture->Release();
	if (m_heightSRV) m_heightSRV->Release();
	if (m_heightUAV) m_heightUAV->Release();
	if (m_originalHeightTexture) m_originalHeightTexture->Release();
	if (m_decalTexture) m_decalTexture->Release();
	if (m_decalSRV) m_decalSRV->Release();
	if (m_decalUAV) m_decalUAV->Release();
	if (m_deformationCS) m_deformationCS->Release();
	if (m_decalCS) m_decalCS->Release();
	if (m_hoofPrintBuffer) m_hoofPrintBuffer->Release();
	if (m_hoofPrintSRV) m_hoofPrintSRV->Release();
	if (m_deformationParamsBuffer) m_deformationParamsBuffer->Release();
}

void TerrainDeformation::AddHoofPrint(const XMFLOAT3& position, const XMFLOAT3& normal, float force, float size, float angle) {
	HoofPrint hoofPrint = {};
	hoofPrint.position = position;
	hoofPrint.normal = normal;
	hoofPrint.depth = m_maxDeformationDepth * force;
	hoofPrint.size = size;
	hoofPrint.angle = angle;
	hoofPrint.timestamp = static_cast<float>(GetTickCount64()) / 1000.0f; // Convert to seconds
	hoofPrint.isActive = true;

	m_hoofPrints.push_back(hoofPrint);

	//古い蹄跡を削除
	if (m_hoofPrints.size() > 1000) {
		m_hoofPrints.erase(m_hoofPrints.begin());
	}
}

void TerrainDeformation::Update(double deltaTime) {
	float currentTime = static_cast<float>(GetTickCount64()) / 1000.0f; // Convert to seconds

	//古い蹄跡のクリーンアップ
	CleanupOldHoofPrints(currentTime);

	//蹄跡バッファの更新
	UpdateHoofPrintBuffer();

	//変形パラメータの更新
	DeformationParams params = {};
	params.terrainSize = XMFLOAT4(m_terrainWidth, m_terrainHeight, 1.0f / m_terrainWidth, 1.0f / m_terrainHeight);
	params.deformationParams = XMFLOAT4(m_maxDeformationDepth, m_deformationRadius, m_recoverySpeed, currentTime);
	params.numHoofPrints = static_cast<int>(m_hoofPrints.size());
	params.deltaTime = deltaTime;

	RENDERER.GetDeviceContext()->UpdateSubresource(
		m_deformationParamsBuffer,
		0,
		nullptr,
		&params,
		0,
		0
	);
}

void TerrainDeformation::ApplyDeformation() {
	auto context = RENDERER.GetDeviceContext();

	//コンピュートシェーダーの設定
	context->CSSetShader(m_deformationCS, nullptr, 0);

	//定数バッファの設定
	context->CSSetConstantBuffers(6, 1, &m_deformationParamsBuffer);

	//シェーダーリソースの設定
	ID3D11ShaderResourceView* srvs[] = {nullptr, m_hoofPrintSRV};
	context->CSSetShaderResources(0, 2, srvs);

	//アンオーダードアクセスビューの設定
	ID3D11UnorderedAccessView* uavs[] = {m_heightUAV, m_decalUAV};
	context->CSSetUnorderedAccessViews(0, 2, uavs, nullptr);

	//ディスパッチ
	UINT groupsX = (m_heightMapWidth + 7) / 8;
	UINT groupsY = (m_heightMapHeight + 7) / 8;
	context->Dispatch(groupsX, groupsY, 1);

	//リソースの解放
	ID3D11ShaderResourceView* nullSRV[] = {nullptr, nullptr};
	context->CSSetShaderResources(0, 2, nullSRV);
	ID3D11UnorderedAccessView* nullUAV[] = {nullptr, nullptr};
	context->CSSetUnorderedAccessViews(0, 2, nullUAV, nullptr);
}

void TerrainDeformation::SetDeformationParams(float maxDepth, float radius, float recoverySpeed) {
	m_maxDeformationDepth = maxDepth;
	m_deformationRadius = radius;
	m_recoverySpeed = recoverySpeed;
}

void TerrainDeformation::ResetTerrain() {
	auto context = RENDERER.GetDeviceContext();

	//ハイトマップをオリジナルにリセット
	context->CopyResource(m_heightTexture, m_originalHeightTexture);

	//デカールテクスチャをクリア
	float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	context->ClearUnorderedAccessViewFloat(m_decalUAV, clearColor);

	//蹄跡のクリア
	m_hoofPrints.clear();
}

bool TerrainDeformation::CreateHeightTextures() {
	HRESULT hr;
	auto device = RENDERER.GetDevice();

	// ハイトマップテクスチャの作成
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = m_heightMapWidth;
	textureDesc.Height = m_heightMapHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	hr = device->CreateTexture2D(&textureDesc, nullptr, &m_heightTexture);
	if (FAILED(hr)) {
		ErrorMessage(L"テクスチャの作成に失敗しました。", hr);
		return false;
	}

	// シェーダーリソースビューの作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(m_heightTexture, &srvDesc, &m_heightSRV);

	return true;
}
