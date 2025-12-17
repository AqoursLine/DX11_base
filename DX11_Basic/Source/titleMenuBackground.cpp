#include "titleMenuBackground.h"
#include "renderer.h"
#include "shaders.h"
#ifdef _DEBUG
#include "imguiSystem.h"
#endif // _DEBUG


bool TitleManuBackground::Initialize() {
	//頂点シェーダー読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitColorVS.cso");
	//ピクセルシェーダー読み込み
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitColorPS.cso");

	// メッシュ生成
	VERTEX_3D mesh[4] = {};

	for (int i = 0; i < 4; i++) {
		mesh[i].diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		mesh[i].normal = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
		mesh[i].tangent = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
		mesh[i].texcoord = XMFLOAT4((i % 2) * 1.0f, (i / 2) * 1.0f, 0.0f, 0.0f);
	}
	mesh[0].position = XMFLOAT4(SCREEN_WIDTH * 0.8f, 0.0f, 0.0f, 1.0f);
	mesh[1].position = XMFLOAT4(SCREEN_WIDTH, 0.0f, 0.0f, 1.0f);
	mesh[2].position = XMFLOAT4(SCREEN_WIDTH * 0.7f, SCREEN_HEIGHT, 0.0f, 1.0f);
	mesh[3].position = XMFLOAT4(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f);

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = sizeof(VERTEX_3D) * 4;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = mesh;
	HRESULT hr = RENDERER.GetDevice()->CreateBuffer(&vbDesc, &vbData, m_vertexBuffer.GetAddressOf());
	if (FAILED(hr)) {
		ErrorMessage(L"タイトルメニュー背景の頂点バッファの作成に失敗しました。", hr);
		return false;
	}

	m_color = XMFLOAT4(0.0f, 0.8f, 1.0f, 0.7f);

	return true;
}

void TitleManuBackground::Finalize() {
	delete m_pixelShader;
	m_pixelShader = nullptr;
	delete m_vertexShader;
	m_vertexShader = nullptr;
}

void TitleManuBackground::Update(double deltaTime) {}

void TitleManuBackground::Draw() {

	auto context = RENDERER.GetDeviceContext();

	//ワールド行列設定
	RENDERER.SetWorldMatrix(XMMatrixIdentity());

	//シェーダーの設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	//マテリアル設定
	MATERIAL material = {};
#ifdef _DEBUG
	//ImGuiで色変更
	ImGui::Begin("TitleMenuBackground Color");
	ImGui::ColorEdit4("Color", (float*)&m_color, ImGuiColorEditFlags_Float);
	ImGui::End();
#endif // _DEBUG

	material.diffuse = m_color;
	RENDERER.SetMaterial(material);

	//頂点バッファの設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	//プリミティブトポロジの設定（三角形リスト）
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//描画
	context->Draw(4, 0);
}
