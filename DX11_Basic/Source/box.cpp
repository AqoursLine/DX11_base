#include "main.h"
#include "box.h"
#include "renderer.h"
#include "texture.h"

//スタティックメンバーの初期化
ComPtr<ID3D11Buffer> Box::m_vertexBuffer = nullptr;
ComPtr<ID3D11Buffer> Box::m_indexBuffer = nullptr;
UINT Box::m_numIndices = 0;
int Box::m_refCount = 0;

bool Box::Initialize() {
	//参照カウントを増やす
	m_refCount++;
	//参照カウントが1より大きければ初期化済み
	if (m_refCount > 1) {
		return true;
	}

	//1辺が1の立方体の頂点データ
	VERTEX_3D vertices[] = {
		//前面
		{{-0.5f, 0.5f, -0.5f, 1.0f},	{0.0f, 0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},	//左上
		{{0.5f, 0.5f, -0.5f, 1.0f},		{0.0f, 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},	//右上
		{{-0.5f, -0.5f, -0.5f, 1.0f},	{0.0f, 0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},	//左下
		{{0.5f, -0.5f, -0.5f, 1.0f},	{0.0f, 0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},	//右下
		//背面
		{{-0.5f, 0.5f, 0.5f, 1.0f},		{0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f, 1.0}, {0.0f, 1.0f, 0.0f, 1.0}},		//右上
		{{0.5f, 0.5f, 0.5f, 1.0f},		{0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f, 1.0}, {0.0f, 1.0f, 0.0f, 1.0}},		//左上
		{{-0.5f, -0.5f, 0.5f, 1.0f},	{0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f, 1.0}, {0.0f, 1.0f, 0.0f, 1.0}},		//右下
		{{0.5f, -0.5f, 0.5f, 1.0f},		{0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f, 1.0}, {0.0f, 1.0f, 0.0f, 1.0}},		//左下
		//左面
		{{-0.5f, 0.5f, 0.5f, 1.0f},		{-1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0}},	//左上
		{{-0.5f, 0.5f, -0.5f, 1.0f},	{-1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},	//右上
		{{-0.5f, -0.5f, 0.5f, 1.0f},	{-1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},	//左下
		{{-0.5f, -0.5f, -0.5f, 1.0f},	{-1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},	//右下
		//右面
		{{0.5f, 0.5f, 0.5f, 1.0f},		{1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},	//右上
		{{0.5f, 0.5f, -0.5f, 1.0f},		{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0}},		//左上
		{{0.5f, -0.5f, 0.5f, 1.0f},		{1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},	//右下
		{{0.5f, -0.5f, -0.5f, 1.0f},	{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},	//左下
		//上面
		{{-0.5f, 0.5f, 0.5f, 1.0f},		{0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0}},		//左上
		{{0.5f, 0.5f, 0.5f, 1.0f},		{0.0f, 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},	//右上
		{{-0.5f, 0.5f, -0.5f, 1.0f},	{0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},	//左下
		{{0.5f, 0.5f, -0.5f, 1.0f},		{0.0f, 1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},	//右下
		//下面
		{{0.5f, -0.5f, -0.5f, 1.0f},	{0.0f, -1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0}},	//右上
		{{-0.5f, -0.5f, -0.5f, 1.0f},	{0.0f, -1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0}},	//左上
		{{0.5f, -0.5f, 0.5f, 1.0f},		{0.0f, -1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},	//右下
		{{ -0.5f, -0.5f, 0.5f, 1.0f },	{0.0f, -1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}}	//左下
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

	return true;
}

void Box::Finalize() {
	//参照カウントを減らす
	m_refCount--;
	//参照カウントが0になったら解放
	if (m_refCount <= 0) {
		m_vertexBuffer.Reset();
		m_indexBuffer.Reset();
	}
}

void Box::Draw(const Vector3& pos, const Vector3& rot, const Vector3& scale) const {
	//デフォルトサンプラーステートセット
	RENDERER.SetSamplerState();

	//マテリアルセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	material.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.shininess = 32.0f;
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

