#include "water.h"
#include "renderer.h"

Water::Water()
	: m_vertexBuffer(nullptr)
	, m_indexBuffer(nullptr)
	, m_constantBuffer(nullptr)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_inputLayout(nullptr)
	, m_waterSize(1000.0f)		//水面のサイズ
	, m_waveHeight(10.0f)		//波の高さスケール
	, m_time(0.0f)			//経過時間
	, m_gridResolution(200)	//グリッドの解像度
	, m_baseWaveFrequency1(0.02f)	//基本波1の周波数
	, m_baseWaveFrequency2(0.015f)	//基本波2の周波数
	, m_baseWaveFrequency3(0.01f)	//基本波3の周波数
	, m_baseWaveSpeed1(2.0f)		//基本波1の速度
	, m_baseWaveSpeed2(1.5f)		//基本波2の速度
	, m_baseWaveSpeed3(1.0f)		//基本波3の速度
	, m_rippleIndex(0)
	, m_wakeTrailIndex(0)
{
	m_ripples.resize(MAX_RIPPLES);
	m_wakeTrails.resize(MAX_WAKE_TRAILS);
}

Water::~Water() {
	Finalize();

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

void Water::AddWakeTrail(const Vector3& startPos, const Vector3& endPos, float width, float intensity) {
	//次のインデックスに航跡を追加
	WakeTrail& wake = m_wakeTrails[m_wakeTrailIndex];
	wake.startPosition = startPos;
	wake.endPosition = endPos;
	wake.direction = (endPos - startPos);
	wake.direction.Normalize();
	wake.width = width;
	wake.length = (endPos - startPos).Length();
	wake.intensity = intensity;
	wake.time = 0.0f;
	wake.lifetime = 15.0f + wake.length * 0.1f; //長さに応じて寿命を調整
	wake.active = true;

	m_wakeTrailIndex = (m_wakeTrailIndex + 1) % MAX_WAKE_TRAILS;
}

void Water::UpdateBoatWake(const Vector3& currentPos, const Vector3& previousPos, float boatSpeed, float boatWidth) {
	//ボートが十分移動した場合のみ航跡を追加
	Vector3 movement = currentPos - previousPos;
	float moveDistance = movement.Length();

	if (moveDistance > 0.5f && boatSpeed > 0.5f) {
		//速度に応じた航跡の強度と幅
		float speedFactor = std::min(boatSpeed / 50.0f, 2.0f);
		float wakeWidth = boatWidth * 2.0f * speedFactor * 3.0f;
		float wakeIntensity = 0.8f + speedFactor * 1.2f;

		//航跡を追加
		AddWakeTrail(previousPos, currentPos, wakeWidth, wakeIntensity);

		//高速時は追加の側面波も生成
		if (boatSpeed > 20.0f) {
			Vector3 right = movement.Cross(Vector3::UP);
			right.Normalize();

			//左右に小さな波紋を追加
			Vector3 leftWavePos = currentPos - right * (boatWidth * 1.5f);
			Vector3 rightWavePos = currentPos + right * (boatWidth * 1.5f);

			AddRipple(leftWavePos, wakeIntensity * 0.6f, 1.2f, 4.0f);
			AddRipple(rightWavePos, wakeIntensity * 0.6f, 1.2f, 4.0f);
		}
	}
}

void Water::ClearWakeTrails() {
	for (auto& wake : m_wakeTrails) {
		wake.active = false;
	}
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
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}
	if (m_pixelShader) {
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}
	if (m_inputLayout) {
		m_inputLayout->Release();
		m_inputLayout = nullptr;
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

	//航跡更新
	for (auto& wake : m_wakeTrails) {
		if (wake.active) {
			wake.time += dt;
			//一定時間経過したら非アクティブにする
			if (wake.time > wake.lifetime) {
				wake.active = false;
			}
		}
	}
}

void Water::Draw() const {
	if (!m_vertexBuffer || !m_indexBuffer) {
		return;
	}

	ID3D11DeviceContext* context = RENDERER.GetDeviceContext();

	//定数バッファ更新
	const_cast<Water*>(this)->UpdateConstantBuffer();

	//ワールド行列設定
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	world = scale * rot * trans;
	RENDERER.SetWorldMatrix(world);

	//シェーダセット
	context->VSSetShader(m_vertexShader, nullptr, 0);
	context->PSSetShader(m_pixelShader, nullptr, 0);
	context->VSSetConstantBuffers(6, 1, &m_constantBuffer);
	context->PSSetConstantBuffers(6, 1, &m_constantBuffer);

	//入力レイアウトセット
	context->IASetInputLayout(m_inputLayout);

	//頂点バッファセット
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//プリミティブトポロジー設定
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//ブレンドを有効
	RENDERER.SetATCEnable(false);
	RENDERER.SetDepthStencilState(true);

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
			vertex.normal = { 0.0f, 1.0f, 0.0f };
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
	RENDERER.CreateVertexShader(&m_vertexShader, &m_inputLayout, L"Shader\\waterVS.cso");
	if (!m_vertexShader || !m_inputLayout) {
		return false;
	}
	//ピクセルシェーダー作成
	RENDERER.CreatePixelShader(&m_pixelShader, L"Shader\\waterPS.cso");
	if (!m_pixelShader) {
		return false;
	}
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

		//航跡データ設定
		for (int i = 0; i < MAX_WAKE_TRAILS; i++) {
			const WakeTrail& wake = m_wakeTrails[i];
			cb->wakeTrails[i].startPos = XMFLOAT4(wake.startPosition.x, wake.startPosition.y, wake.startPosition.z, wake.time);
			cb->wakeTrails[i].endPos = XMFLOAT4(wake.endPosition.x, wake.endPosition.y, wake.endPosition.z, wake.intensity);
			cb->wakeTrails[i].params = XMFLOAT4(wake.width, wake.length, wake.lifetime, wake.active ? 1.0f : 0.0f);
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

	//航跡効果
	height += CalculateWakeHeight(position, time);

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

float Water::CalculateWakeHeight(const Vector3& position, float time) const {
	float wakeHeight = 0.0f;

	for (const auto& wake : m_wakeTrails) {
		if (!wake.active) {
			continue;
		}

		//航跡線分に対する最接近点を計算
		Vector3 wakeVec = wake.endPosition - wake.startPosition;
		Vector3 pointVec = position - wake.startPosition;

		float wakeLength = wakeVec.Length();
		if (wakeLength < 0.1f) {
			continue;
		}

		Vector3 wakeDir = wakeVec / wakeLength;
		float projLength = pointVec.Dot(wakeDir);

		//航跡の範囲内かチェック
		if (projLength < 0.0f || projLength > wakeLength) {
			continue;
		}

		//航跡線分上の最接近点
		Vector3 closestPoint = wake.startPosition + wakeDir * projLength;

		//最接近点からの距離
		Vector3 offsetVec = position - closestPoint;
		float lateralDistance = offsetVec.Length();

		//幅の範囲内かチェック
		if (lateralDistance > wake.width) {
			continue;
		}

		//V字型波の計算
		float normalizedPos = projLength / wakeLength; //0から1の範囲
		float normalizedLateral = lateralDistance / wake.width; //0から1の範囲

		//時間減衰
		float timeAttenuation = exp(-wake.time * 0.1f);

		//距離減衰
		float lateralAttenuation = cos(normalizedLateral * 3.14159f * 0.5f); //コサインで中央が高くなる

		//長さ方向の減衰
		float lengthAttenuation = 1.0f - normalizedPos * 0.3f;

		//V字パターン
		float kelvinAngle = 19.47f * (3.14159f / 180.0f); //ケルビン角度
		float expectedLateral = normalizedPos * wakeLength * tan(kelvinAngle);

		float kelvinFactor = 1.0f;
		if (lateralDistance > expectedLateral * 0.5f) {
			kelvinFactor = exp(-(lateralDistance - expectedLateral * 0.5f) / wake.width);
		}

		//波の高さ計算
		float wavePhase = (projLength * 0.5f + lateralDistance * 2.0f - time * 3.0f);
		float amplitude = wake.intensity * timeAttenuation * lateralAttenuation * lengthAttenuation * kelvinFactor;

		wakeHeight += std::sin(wavePhase) * amplitude * 0.3f;

		//追加の小さな波紋
		float smallWavePhase = (projLength * 2.0f + lateralDistance * 5.0f - time * 8.0f);
		wakeHeight += std::sin(smallWavePhase) * amplitude * 0.15f;
	}

	return wakeHeight;
}
