#include "resultRankText.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

bool ResultRankText::Initialize() {
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		ErrorMessage(L"リザルトランクテキストのスプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}

	//テクスチャの読み込み
	m_texture = new Texture();
	std::wstring texturePath = L"Asset\\Texture\\resultRank_" + std::to_wstring(m_rankIndex + 1) + L".png";
	if (!m_texture->Load(texturePath)) {
		ErrorMessage(L"リザルトランクテキストのテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}

	//シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	m_scale = { 150.0f, 100.0f, 1.0f };
	m_position = { 500.0f, 400.0f + (m_rankIndex * (m_scale.y + 20.0f)), 0.0f };

	return true;
}

void ResultRankText::Finalize() {
	m_sprite->Finalize();
	delete m_sprite;
	delete m_texture;
	m_texture = nullptr;
	delete m_pixelShader;
	m_pixelShader = nullptr;
	delete m_vertexShader;
	m_vertexShader = nullptr;
}

void ResultRankText::Update(double deltaTime) {
}

void ResultRankText::Draw() {
	// シェーダーの設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// マテリアルセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	// テクスチャの設定
	m_texture->Set(0);

	// 描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
