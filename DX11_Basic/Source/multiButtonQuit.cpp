#include "multiButtonQuit.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

#include "system.h"
#include "manager.h"
#include "titleScene.h"
#include "testTransition.h"

void MultiButtonQuit::OnDecide() {
	if (m_isDecided) {
		return;
	}


	SYSTEM.GetManager()->SetScene(new TitleScene(), new TestTransition());

	m_isDecided = true;
}

bool MultiButtonQuit::Initialize() {
	// スプライト生成
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		return false;
	}
	// テクスチャ作成
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\multiButtonQuit.png")) {
		return false;
	}
	// バーテックスシェーダー作成
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	// ピクセルシェーダー作成
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	SetStartScale(Vector3(250.0f, 100.0f, 1.0f));
	SetTargetScale(Vector3(300.0f, 120.0f, 1.0f));

	return true;
}

void MultiButtonQuit::Finalize() {
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

void MultiButtonQuit::Draw() {
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
