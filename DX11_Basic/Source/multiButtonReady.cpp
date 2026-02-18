#include "multiButtonReady.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

#include "system.h"
#include "webClient.h"

void MultiButtonReady::OnDecide() {
	m_isReady = !m_isReady;

	auto webClient = SYSTEM.GetWebClient();

	webClient->SendMessageClient(json{
		{"type", "guestReady"},
		{"ready", m_isReady},
	});
}

bool MultiButtonReady::Initialize() {
	// スプライト生成
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		return false;
	}
	// テクスチャ作成
	m_readyTexture = new Texture();
	if (!m_readyTexture->Load(L"Asset\\Texture\\multiButtonReady.png")) {
		return false;
	}
	m_cancelTexture = new Texture();
	if (!m_cancelTexture->Load(L"Asset\\Texture\\multiButtonCancel.png")) {
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

void MultiButtonReady::Finalize() {
	// リソースの解放
	if (m_sprite) {
		m_sprite->Finalize();
		delete m_sprite;
		m_sprite = nullptr;
	}
	if (m_readyTexture) {
		delete m_readyTexture;
		m_readyTexture = nullptr;
	}
	if (m_cancelTexture) {
		delete m_cancelTexture;
		m_cancelTexture = nullptr;
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

void MultiButtonReady::Draw() {
	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// テクスチャ設定
	if (m_isReady) {
		m_cancelTexture->Set(0);
	} else {
		m_readyTexture->Set(0);
	}

	// マテリアル設定
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = TRUE;
	RENDERER.SetMaterial(material);

	// スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
