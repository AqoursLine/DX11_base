#include "water.h"
#include "renderer.h"

#include "shaders.h"

Water::Water()
	: m_vertexBuffer(nullptr)
	, m_indexBuffer(nullptr)
	, m_constantBuffer(nullptr)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_waterSize(500)		//水面のサイズ
	, m_waveHeight(2.0f)		//波の高さスケール
	, m_time(0.0f)			//経過時間
	, m_gridResolution(200)	//グリッドの解像度
	, m_baseWaveFrequency1(0.02f)	//基本波1の周波数
	, m_baseWaveFrequency2(0.015f)	//基本波2の周波数
	, m_baseWaveFrequency3(0.01f)	//基本波3の周波数
	, m_baseWaveSpeed1(2.0f)		//基本波1の速度
	, m_baseWaveSpeed2(1.5f)		//基本波2の速度
	, m_baseWaveSpeed3(1.0f)		//基本波3の速度
	, m_rippleIndex(0)
{
	m_ripples.resize(MAX_RIPPLES);
}

Water::~Water() {
}

void Water::AddRipple(const Vector3& position, float amplitude, float frequency, float speed) {
	//次のインデックスに波紋を追加
	Ripple& ripple = m_ripples[m_rippleIndex];
	ripple.position = position;
	ripple.amplitude = amplitude;
	ripple.frequency = frequency;
	ripple.speed = speed;
	ripple.time = 0.0f;
	ripple.active = true;

	m_rippleIndex = (m_rippleIndex + 1) % MAX_RIPPLES;
}

float Water::GetWaterHeight(const Vector3& position) const {
	return m_position.y + CalculateWaveHeight(position, m_time);
}

Vector3 Water::GetWaterNormal(const Vector3& position) const {
	return CalculateWaveNormal(position, m_time);
}

bool Water::Initialize() {
	if (!CreateWaterMesh()) {
		return false;
	}

	if (!CreateShaders()) {
		return false;
	}

	//定数バッファ作成
	D3D11_BUFFER_DESC cbd = {};
	cbd.ByteWidth = sizeof(WaterConstantBuffer);
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = RENDERER.GetDevice()->CreateBuffer(&cbd, nullptr, &m_constantBuffer);
	if (FAILED(hr)) {
		return false;
	}

	m_scale = { 1.0f, 1.0f, 1.0f };

	return true;
}

void Water::Finalize() {
	if (m_vertexBuffer) {
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
	if (m_indexBuffer) {
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}
	if (m_constantBuffer) {
		m_constantBuffer->Release();
		m_constantBuffer = nullptr;
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

void Water::Update(double deltaTime) {
	float dt = static_cast<float>(deltaTime);

	m_time += dt;

	//波紋更新
	for (auto& ripple : m_ripples) {
		if (ripple.active) {
			ripple.time += dt;
			//一定時間経過したら非アクティブにする
			if (ripple.time > 5.0f) {
				ripple.active = false;
			}
		}
	}
}

void Water::Draw() const {
	if (!m_vertexBuffer || !m_indexBuffer) {
		return;
	}

	ID3D11DeviceContext* context = RENDERER.GetDeviceContext();

	//シェーダセット
	m_vertexShader->Set();
	m_pixelShader->Set();

	//定数バッファ更新
	const_cast<Water*>(this)->UpdateConstantBuffer();

	//ワールド行列設定
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;
	RENDERER.SetWorldMatrix(world);

	context->VSSetConstantBuffers(7, 1, &m_constantBuffer);
	context->PSSetConstantBuffers(7, 1, &m_constantBuffer);

	//マテリアル
	MATERIAL material = {};
	material.ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
	RENDERER.SetMaterial(material);

	//頂点バッファセット
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//プリミティブトポロジー設定
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//描画
	UINT indexCount = (m_gridResolution - 1) * (m_gridResolution - 1) * 6;
	context->DrawIndexed(indexCount, 0, 0);

}

bool Water::CreateWaterMesh() {
	//頂点データ生成
	std::vector<VERTEX_3D> vertices;
	vertices.reserve(m_gridResolution * m_gridResolution);

	float halfSize = m_waterSize * 0.5f;
	float step = m_waterSize / (m_gridResolution - 1);

	for (int z = 0; z < m_gridResolution; z++) {
		for (int x = 0; x < m_gridResolution; x++) {
			VERTEX_3D vertex = {};
			vertex.position.x = -halfSize + x * step;
			vertex.position.y = 0.0f;
			vertex.position.z = -halfSize + z * step;
			vertex.position.w = 1.0f;
			vertex.normal = { 0.0f, 1.0f, 0.0f, 1.0f };
			vertex.diffuse = { 0.2f, 0.8f, 1.0f, 0.8f };
			vertex.texcoord.x = static_cast<float>(x) / (m_gridResolution - 1);
			vertex.texcoord.y = static_cast<float>(z) / (m_gridResolution - 1);
			vertices.push_back(vertex);
		}
	}

	//インデックスデータ生成
	std::vector<UINT> indices;
	indices.reserve((m_gridResolution - 1) * (m_gridResolution - 1) * 6);

	for (int z = 0; z < m_gridResolution - 1; z++) {
		for (int x = 0; x  < m_gridResolution - 1; x ++) {
			UINT topLeft = z * m_gridResolution + x;
			UINT topRight = topLeft + 1;
			UINT bottomLeft = (z + 1) * m_gridResolution + x;
			UINT bottomRight = bottomLeft + 1;

			//三角形1
			indices.push_back(topLeft);
			indices.push_back(bottomLeft);
			indices.push_back(topRight);

			//三角形2
			indices.push_back(topRight);
			indices.push_back(bottomLeft);
			indices.push_back(bottomRight);
		}
	}

	//頂点バッファ作成
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = sizeof(VERTEX_3D) * static_cast<UINT>(vertices.size());
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices.data();

	HRESULT hr = RENDERER.GetDevice()->CreateBuffer(&bd, &initData, &m_vertexBuffer);
	if (FAILED(hr)) {
		return false;
	}

	//インデックスバッファ作成
	bd = {};
	bd.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices.size());
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	initData = {};
	initData.pSysMem = indices.data();

	hr = RENDERER.GetDevice()->CreateBuffer(&bd, &initData, &m_indexBuffer);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

bool Water::CreateShaders() {
	//頂点シェーダー作成
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\waterVS.cso");

	//ピクセルシェーダー作成
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\waterPS.cso");
	return true;
}

void Water::UpdateConstantBuffer() {
	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = RENDERER.GetDeviceContext()->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	if (SUCCEEDED(hr)) {
		WaterConstantBuffer* cb = static_cast<WaterConstantBuffer*>(mapped.pData);

		cb->time = m_time;
		cb->waveHeight = m_waveHeight;
		cb->waterSize = m_waterSize;
		cb->padding = 0.0f;

		//基本波パラメータ設定
		cb->baseWaveFreq1 = m_baseWaveFrequency1;
		cb->baseWaveFreq2 = m_baseWaveFrequency2;
		cb->baseWaveFreq3 = m_baseWaveFrequency3;
		cb->baseWaveSpeed1 = m_baseWaveSpeed1;
		cb->baseWaveSpeed2 = m_baseWaveSpeed2;
		cb->baseWaveSpeed3 = m_baseWaveSpeed3;

		//波紋データ設定
		for (int i = 0; i < MAX_RIPPLES; i++) {
			const Ripple& ripple = m_ripples[i];
			cb->ripples[i].positionAndTime = XMFLOAT4(ripple.position.x, ripple.position.y, ripple.position.z, ripple.time);
			cb->ripples[i].params = XMFLOAT4(ripple.amplitude, ripple.frequency, ripple.speed, ripple.active ? 1.0f : 0.0f);
		}

		RENDERER.GetDeviceContext()->Unmap(m_constantBuffer, 0);
	}
}

float Water::CalculateWaveHeight(const Vector3& position, float time) const {
	float height = 0.0f;

	//基本的な波
	float x = position.x;
	float z = position.z;

	height += std::sin(x * m_baseWaveFrequency1 + time * m_baseWaveSpeed1) * m_waveHeight * 0.3f;
	height += std::sin(z * m_baseWaveFrequency2 + time * m_baseWaveSpeed2) * m_waveHeight * 0.2f;
	height += std::sin((x + z) * m_baseWaveFrequency3 + time * m_baseWaveSpeed3) * m_waveHeight * 0.5f;

	//波紋効果
	for (const auto& ripple : m_ripples) {
		if (!ripple.active) {
			continue;
		}

		float dx = position.x - ripple.position.x;
		float dz = position.z - ripple.position.z;
		float distance = std::sqrt(dx * dx + dz * dz);

		if (distance < ripple.speed * ripple.time) {
			float wavePhase = ripple.frequency * (distance - ripple.speed * ripple.time);
			float attenuation = exp(-ripple.time * 0.5f); //時間減衰
			float distanceAttenuation = 1.0f / (1.0f + distance * 0.01f); //距離減衰
			height += std::sin(wavePhase) * ripple.amplitude * attenuation * distanceAttenuation;
		}
	}

	return height;
}

Vector3 Water::CalculateWaveNormal(const Vector3& position, float time) const {
	const float epsilon = 0.1f;

	//周囲4点の高さを取得
	float heightL = CalculateWaveHeight(Vector3(position.x - epsilon, position.y, position.z), time);
	float heightR = CalculateWaveHeight(Vector3(position.x + epsilon, position.y, position.z), time);
	float heightD = CalculateWaveHeight(Vector3(position.x, position.y, position.z - epsilon), time);
	float heightU = CalculateWaveHeight(Vector3(position.x, position.y, position.z + epsilon), time);

	//高さの差から法線を計算
	Vector3 normal;
	normal.x = (heightL - heightR) / (2.0f * epsilon);
	normal.y = 1.0f;
	normal.z = (heightD - heightU) / (2.0f * epsilon);

	normal.Normalize();
	return normal;
}
