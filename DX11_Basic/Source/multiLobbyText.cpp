#include "multiLobbyText.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"


bool MultiLobbyText::Initialize() {
	// スプライト生成
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		return false;
	}

	// テクスチャ作成
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\multiLobbyText.png")) {
		return false;
	}

	// バーテックスシェーダー作成
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	// ピクセルシェーダー作成
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	// トランスフォームの初期化
	m_position = Vector3(SCREEN_WIDTH * 0.5f, 150.0f, 0.0f);
	m_rotation = Vector3::ZERO;
	m_scale = Vector3(1200.0f, 250.0f, 1.0f);


	return true;
}

void MultiLobbyText::Finalize() {
	// リソースの解放
	if (m_sprite) {
		m_sprite->Finalize();
		delete m_sprite;
		m_sprite = nullptr;
	}
	if (m_texture) {
		delete m_texture;
		m_texture = nullptr;
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

void MultiLobbyText::Draw() {
	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();
	// テクスチャ設定
	m_texture->Set(0);
	// マテリアル設定
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = TRUE;
	RENDERER.SetMaterial(material);
	// スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
