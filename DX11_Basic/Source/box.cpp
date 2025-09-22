#include "main.h"
#include "box.h"
#include "renderer.h"
#include "texture.h"


bool Box::Initialize() {
	//1辺が1の立方体の頂点データ
	VERTEX_3D vertices[] = {
		//前
		{{-0.5f, 0.5f, -0.5f},	{0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f},	{0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{-0.5f, -0.5f, -0.5f},	{0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{0.5f, -0.5f, -0.5f},	{0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		//後
		{{-0.5f, 0.5f, 0.5f},	{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, 0.5f},	{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{-0.5f, -0.5f, 0.5f},	{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{0.5f, -0.5f, 0.5f},	{0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		//左
		{{-0.5f, 0.5f, 0.5f},	{-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{-0.5f, 0.5f, -0.5f},	{-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		{{-0.5f, -0.5f, 0.5f},	{-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f, -0.5f},	{-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
		//右
		{{0.5f, 0.5f, 0.5f},	{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f},	{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.5f},	{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{0.5f, -0.5f, -0.5f},	{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		//上
		{{-0.5f, 0.5f, 0.5f},	{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.5f},	{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		{{-0.5f, 0.5f, -0.5f},	{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{0.5f, 0.5f, -0.5f},	{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
		//下
		{{-0.5f, -0.5f, 0.5f},	{0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
		{{0.5f, -0.5f, 0.5f},	{0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f, -0.5f},	{0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f},	{0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
	};

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

	//インデックスデータ(時計周り)
	UINT indices[] = {
		//前
		0, 1, 2,
		1, 3, 2,
		//後
		4, 6, 5,
		5, 6, 7,
		//左
		8, 9, 10,
		9, 11, 10,
		//右
		12, 14, 13,
		13, 14, 15,
		//上
		16, 17, 18,
		17, 19, 18,
		//下
		20, 22, 21,
		21, 22, 23
	};

	//インデックスバッファの設定
	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA indexInitData = {};
	indexInitData.pSysMem = indices;
	hr = RENDERER.GetDevice()->CreateBuffer(&indexBufferDesc, &indexInitData, m_indexBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage((L"インデックスバッファの初期化に失敗しました。"), hr);
		return false;
	}

	//インデックスカウントを保存
	m_numIndices = _countof(indices);

	//シェーダーの作成
	RENDERER.CreateVertexShader(&m_vertexShader, &m_inputLayout, L"Shader\\unlitTextureVS.cso");
	RENDERER.CreatePixelShader(&m_pixelShader, L"Shader\\unlitTexturePS.cso");

	return true;
}

void Box::Finalize() {
}

void Box::Draw(const Vector3& pos, const Vector3& rot, const Vector3& scale) const {
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
	material.textureEnable = false;
	RENDERER.SetMaterial(material);

	// 描画のための行列を設定
	XMMATRIX worldMatrix = XMMatrixScaling(scale.x, scale.y, scale.z) *
		XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) *
		XMMatrixTranslation(pos.x, pos.y, pos.z);
	// ワールド行列をセット
	RENDERER.SetWorldMatrix(worldMatrix);

	// プリミティブトポロジをセット
	RENDERER.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 頂点バッファをセット
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	RENDERER.GetDeviceContext()->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	// インデックスバッファをセット
	RENDERER.GetDeviceContext()->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// インデックスを使って描画
	RENDERER.GetDeviceContext()->DrawIndexed(m_numIndices, 0, 0);

}

// クォータニオン版
void Box::Draw(const Vector3& pos, const Vector4& rot, const Vector3& scale) const {
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
	material.textureEnable = false;
	RENDERER.SetMaterial(material);

	// クォータニオンを回転行列に変換
	XMMATRIX rotMatrix = XMMatrixRotationQuaternion(XMVectorSet(rot.x, rot.y, rot.z, rot.w));
	// ワールド行列を設定
	XMMATRIX worldMatrix, scaleMatrix, posMatrix;	//単位行列
	scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);	//スケーリング
	posMatrix = XMMatrixTranslation(pos.x, pos.y, pos.z);	//平行移動
	worldMatrix = scaleMatrix * rotMatrix * posMatrix;	//ワールド行列を計算
	RENDERER.SetWorldMatrix(worldMatrix);	//ワールド行列をセット

	// プリミティブトポロジをセット
	RENDERER.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 頂点バッファをセット
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	RENDERER.GetDeviceContext()->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	// インデックスバッファをセット
	RENDERER.GetDeviceContext()->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// インデックスを使って描画
	RENDERER.GetDeviceContext()->DrawIndexed(m_numIndices, 0, 0);

}

