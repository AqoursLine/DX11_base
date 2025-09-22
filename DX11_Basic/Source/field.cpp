#include "main.h"
#include "field.h"
#include "renderer.h"
#include "texture.h"


bool Field::Initialize(std::wstring fileName) {
	//テクスチャ読み込み
	m_texture = new Texture();
	if (!m_texture->Load(fileName)) {
		ErrorMessage(L"フィールドのテクスチャ読み込みに失敗しました。", E_FAIL);
		return false;
	}

	//頂点データの作成
	VERTEX_3D vertices[4] = {};

	//三次元ポリゴンの座標を設定
	vertices[0].position = XMFLOAT3(-0.5f, 0.0f, 0.5f);
	vertices[1].position = XMFLOAT3(0.5f, 0.0f, 0.5f);
	vertices[2].position = XMFLOAT3(-0.5f, 0.0f, -0.5f);
	vertices[3].position = XMFLOAT3(0.5f, 0.0f, -0.5f);

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
	vertices[2].diffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
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

	//シェーダーの作成
	RENDERER.CreateVertexShader(&m_vertexShader, &m_inputLayout, L"Shader\\unlitTextureVS.cso");
	RENDERER.CreatePixelShader(&m_pixelShader, L"Shader\\unlitTexturePS.cso");

	return true;
}

void Field::Finalize() {
	// テクスチャの解放
	if (m_texture) {
		delete m_texture;
		m_texture = nullptr;
	}

}

void Field::Draw(const Vector3& pos, const Vector3& rot, const Vector3& scale) const {
	// 入力レイアウトをセット
	RENDERER.GetDeviceContext()->IASetInputLayout(m_inputLayout.Get());
	// 頂点シェーダーをセット
	RENDERER.GetDeviceContext()->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	// ピクセルシェーダーをセット
	RENDERER.GetDeviceContext()->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	//デフォルトサンプラーステートセット
	RENDERER.SetSamplerState();

	//マテリアルセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	// 描画のための行列を設定
	XMMATRIX worldMatrix = XMMatrixScaling(scale.x, scale.y, scale.z) *
		XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) *
		XMMatrixTranslation(pos.x, pos.y, pos.z);
	// ワールド行列をセット
	RENDERER.SetWorldMatrix(worldMatrix);

	// 頂点バッファをセット
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	RENDERER.GetDeviceContext()->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	//テクスチャセット
	RENDERER.GetDeviceContext()->PSSetShaderResources(0, 1, m_texture->GetTextureAddress());

	// プリミティブトポロジをセット
	RENDERER.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 描画
	RENDERER.GetDeviceContext()->Draw(4, 0);
}

