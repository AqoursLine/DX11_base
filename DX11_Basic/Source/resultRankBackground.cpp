#include "resultRankBackground.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

bool ResultRankBackground::Initialize() {
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		ErrorMessage(L"リザルトランク背景のスプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}

	//テクスチャの読み込み
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\rankBackground.png")) {
		ErrorMessage(L"リザルトランク背景のテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}

	//シェーダーの読み込み
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	m_scale = { 1100.0f, 100.0f, 1.0f };
	m_position = { SCREEN_WIDTH * 0.5f, 400.0f, 0.0f };

	return true;
}

void ResultRankBackground::Finalize() {
	m_sprite->Finalize();
	delete m_sprite;
	delete m_texture;
	m_texture = nullptr;
	delete m_pixelShader;
	m_pixelShader = nullptr;
	delete m_vertexShader;
	m_vertexShader = nullptr;
}

void ResultRankBackground::Update(double deltaTime) {
}

void ResultRankBackground::Draw() {
	// シェーダーの設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// テクスチャの設定
	m_texture->Set(0);

	// マテリアル
	MATERIAL material = {};
	material.textureEnable = true;

	// ポジション
	Vector3 pos = m_position;

	// 複数表示
	for (int i = 0; i < m_resultCount; i++) {
		// マテリアルセット
		// メインプレイヤー
		if (m_resultData.isMainPlayer) {
			material.diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
		} else {
			material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		RENDERER.SetMaterial(material);

	}

}
