#include "titleMenuIcon.h"
#include <algorithm>
#include "renderer.h"
#include "sprite.h"
#include "shaders.h"
#include "texture.h"

#include "imguiSystem.h"

bool TitleMenuIcon::Initialize() {
	// スプライトの初期化
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		return false;
	}
	// テクスチャの読み込み
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\" + m_textureFilePath)) {
		return false;
	}

	// シェーダーの初期化
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");

	// スケールベクトルの設定
	m_baseScaleVector = Vector3(500.0f, 150.0f, 1.0f); // 基本スケールをアイコンのサイズに設定

	// 位置をスケール分左に移動
	m_position.x = m_position.x - (m_baseScaleVector.x * 0.5f);

	// 基本位置とスケールの設定
	m_basePosition = m_position;
	m_currentScale = m_baseScale;


	return true;
}

void TitleMenuIcon::Finalize() {
	// スプライトの終了処理
	if (m_sprite) {
		m_sprite->Finalize();
		delete m_sprite;
		m_sprite = nullptr;
	}
	// テクスチャの解放
	if (m_texture) {
		delete m_texture;
		m_texture = nullptr;
	}

	// シェーダーの解放
	if (m_vertexShader) {
		delete m_vertexShader;
		m_vertexShader = nullptr;
	}
	if (m_pixelShader) {
		delete m_pixelShader;
		m_pixelShader = nullptr;
	}
}

void TitleMenuIcon::Update(double deltaTime) {
		if (m_isSelected) {
			// 選択されている場合、選択スケールに向かって補間
			m_currentScale += (m_selectedScale - m_currentScale) * static_cast<float>(m_scaleLerpSpeed * deltaTime);
		} else {
			// 選択されていない場合、基本スケールに向かって補間
			m_currentScale += (m_baseScale - m_currentScale) * static_cast<float>(m_scaleLerpSpeed * deltaTime);
		}

		// スケールの範囲を制限
		m_currentScale = std::clamp(m_currentScale, m_baseScale, m_selectedScale);

		// スケールを更新
		m_scale = m_baseScaleVector * m_currentScale;

		// 位置を更新(スケール分左に移動)
		float offsetX = (m_scale.x - m_baseScaleVector.x) * 0.5f;
		m_position = Vector3(m_basePosition.x - offsetX, m_basePosition.y, m_basePosition.z);
}

void TitleMenuIcon::Draw() {
	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// テクスチャセット
	m_texture->Set(0);

	// マテリアル設定
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = TRUE;
	RENDERER.SetMaterial(material);

	// スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
