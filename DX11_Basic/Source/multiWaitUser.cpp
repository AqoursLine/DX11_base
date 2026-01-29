#include "multiWaitUser.h"
#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

bool MultiWaitUser::Initialize() {
	// スプライト生成
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		return false;
	}

	// テクスチャ作成
	m_multiIconBackgroundTexture = new Texture();
	if (!m_multiIconBackgroundTexture->Load(L"Asset\\Texture\\multiIconBackground.png")) {
		return false;
	}


	m_readyTexture = new Texture();
	if (!m_readyTexture->Load(L"Asset\\Texture\\ready.png")) {
		return false;
	}
	m_notReadyTexture = new Texture();
	if (!m_notReadyTexture->Load(L"Asset\\Texture\\notReady.png")) {
		return false;
	}

	// バーテックスシェーダー作成
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");

	// ピクセルシェーダー作成
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	return true;
}

void MultiWaitUser::Finalize() {
	// スプライト解放
	if (m_sprite) {
		m_sprite->Finalize();
		delete m_sprite;
		m_sprite = nullptr;
	}
	// テクスチャ解放
	if (m_multiIconBackgroundTexture) {
		delete m_multiIconBackgroundTexture;
		m_multiIconBackgroundTexture = nullptr;
	}
	if (m_readyTexture) {
		delete m_readyTexture;
		m_readyTexture = nullptr;
	}
	if (m_notReadyTexture) {
		delete m_notReadyTexture;
		m_notReadyTexture = nullptr;
	}
	// シェーダー解放
	if (m_vertexShader) {
		delete m_vertexShader;
		m_vertexShader = nullptr;
	}
	if (m_pixelShader) {
		delete m_pixelShader;
		m_pixelShader = nullptr;
	}
}

void MultiWaitUser::Draw() {
	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// テクスチャ設定
	m_multiIconBackgroundTexture->Set(0);

	// カラー設定
	XMFLOAT4 color = XMFLOAT4(m_color.x, m_color.y, m_color.z, m_color.w);
	// アイコン非表示なら暗くする
	if (!m_isIconVisible) {
		color = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.9f);
	}

	// マテリアル設定
	MATERIAL material = {};
	material.diffuse = color;
	material.textureEnable = TRUE;
	RENDERER.SetMaterial(material);

	// スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);

	//====準備状態表示====
	// アイコン非表示なら表示しない
	if (!m_isIconVisible) {
		return;
	}
	// 座標設定
	Vector3 iconPos = m_position + Vector3(130.0f, 0.0f, 0.0f);
	Vector3 iconScale = Vector3(100.0f, 100.0f, 1.0f);

	// テクスチャ設定
	if (m_isReady) {
		m_readyTexture->Set(0);
	} else {
		m_notReadyTexture->Set(0);
	}

	// マテリアル設定
	MATERIAL iconMaterial = {};
	iconMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	iconMaterial.textureEnable = TRUE;
	RENDERER.SetMaterial(iconMaterial);

	// スプライト描画
	m_sprite->Draw(iconPos, Vector3::ZERO, iconScale);
}
