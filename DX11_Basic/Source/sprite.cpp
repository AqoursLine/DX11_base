#include "main.h"
#include "renderer.h"
#include "sprite.h"

//静的メンバ変数の実体
ComPtr<ID3D11Buffer> Sprite::m_vertexBuffer = nullptr;
int Sprite::m_refCount = 0;

bool Sprite::Initialize() {

	//参照カウントを増やす
	m_refCount++;
	if (m_refCount > 1) {
		return true;
	}

	//頂点データの作成
	VERTEX_3D vertices[4] = {};

	//二次元ポリゴン設定
	vertices[0].position = XMFLOAT3(-0.5f, -0.5f, 0.0f);
	vertices[1].position = XMFLOAT3(0.5f, -0.5f, 0.0f);
	vertices[2].position = XMFLOAT3(-0.5f, 0.5f, 0.0f);
	vertices[3].position = XMFLOAT3(0.5f, 0.5f, 0.0f);

	vertices[0].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertices[1].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertices[2].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertices[3].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);

	vertices[0].texcoord = XMFLOAT2(0.0f, 0.0f);
	vertices[1].texcoord = XMFLOAT2(1.0f, 0.0f);
	vertices[2].texcoord = XMFLOAT2(0.0f, 1.0f);
	vertices[3].texcoord = XMFLOAT2(1.0f, 1.0f);

	vertices[0].diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertices[1].diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertices[2].diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertices[3].diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	//頂点バッファの設定
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	HRESULT hr = RENDERER.GetDevice()->CreateBuffer(&bufferDesc, &initData, m_vertexBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"頂点バッファの初期化に失敗しました。", hr);
		return false;
	}

	return true;
}

void Sprite::Finalize() {
	//参照カウントを減らす
	m_refCount--;
	if (m_refCount <= 0) {
		//頂点バッファ解放
		m_vertexBuffer.Reset();
	}
}

void Sprite::Draw(const Vector3& pos, const Vector3& rot, const Vector3& scale) const {
	//デフォルトのサンプラーステートセット
	RENDERER.SetSamplerState();

	// 2D描画のための行列を設定
	XMMATRIX worldMatrix = XMMatrixScaling(scale.x, scale.y, scale.z) *
		XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) *
		XMMatrixTranslation(pos.x, pos.y, pos.z);
	// ワールド行列をセット
	RENDERER.SetWorldMatrix(worldMatrix);

	// 頂点バッファをセット
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	RENDERER.GetDeviceContext()->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	// プリミティブトポロジをセット
	RENDERER.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 描画
	RENDERER.GetDeviceContext()->Draw(4, 0);
}
