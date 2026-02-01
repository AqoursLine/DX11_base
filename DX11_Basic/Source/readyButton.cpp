#include "readyButton.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

bool ReadyButton::Initialize() {
	// スプライト生成
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		return false;
	}
	// テクスチャ作成
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\readyButton.png")) {
		return false;
	}
	// バーテックスシェーダー作成
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	// ピクセルシェーダー作成
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	m_scale = Vector3(250.0f, 100.0f, 1.0f);

	return true;
}

void ReadyButton::Finalize() {}

void ReadyButton::Update(double deltaTime) {}

void ReadyButton::Draw() {
	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// テクスチャ設定
	m_texture->Set(0);

	// マテリアル設定
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	if (!m_isReady) {
		material.diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	}
	material.textureEnable = TRUE;
	RENDERER.SetMaterial(material);

	// スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
