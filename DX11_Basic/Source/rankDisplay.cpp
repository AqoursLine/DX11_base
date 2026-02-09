#include "main.h"
#include "rankDisplay.h"
#include "sprite.h"
#include "renderer.h"
#include "texture.h"
#include "shaders.h"

#include "scene.h"
#include "player.h"
#include "raceManager.h"

bool RankDisplay::Initialize() {
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\unlitTexturePS.cso");
	m_backPixelShader = new PixelShader();
	m_backPixelShader->Load(L"Shader\\roundedRectPS.cso");

	// ランクテクスチャ読み込み
	for (int i = 0; i < MAX_RANKS; i++) {
		m_rankTextures[i] = new Texture();
		if (!m_rankTextures[i]) {
			return false;
		}
		std::wstring texturePath = L"Asset\\Texture\\rank_" + std::to_wstring(i + 1) + L".png";
		if (!m_rankTextures[i]->Load(texturePath.c_str())) {
			return false;
		}
	}

	m_sprite = new Sprite();
	m_sprite->Initialize();

	m_scale = { 120.f, 120.0f, 1.0f };
	m_position = { 20.0f + m_scale.x * 0.5f, SCREEN_HEIGHT - m_scale.y * 0.5f - 20.0f, 0.0f };

	m_player = m_scene->GetGameObject<Player>();

	if (!m_player) {
		SetActive(false);
	}

	return true;
}

void RankDisplay::Finalize() {
	// テクスチャ解放
	for (int i = 0; i < MAX_RANKS; i++) {
		if (m_rankTextures[i]) {
			delete m_rankTextures[i];
		}
	}


	// スプライト解放
	if (m_sprite) {
		m_sprite->Finalize();
		delete m_sprite;
	}

	// シェーダー解放
	if (m_vertexShader) {
		delete m_vertexShader;
	}
	if (m_pixelShader) {
		delete m_pixelShader;
	}
	if (m_backPixelShader) {
		delete m_backPixelShader;
	}
}

void RankDisplay::Update(double deltaTime) {
	float playerProgress = m_player->GetLapProgress();
	int playerLap = m_player->GetLapCount();

	if (playerLap == m_scene->GetGameObject<RaceManager>()->GetLapCountToFinish()) {
		if (playerProgress >= m_fadeStartProgress) {
			m_alpha -= static_cast<float>(deltaTime) / m_fadeDuration;
			m_alpha = std::max(m_alpha, 0.0f);
		}
	}
}

void RankDisplay::Draw() {
	// 背景描画
	//シェーダーの設定
	m_vertexShader->Set();
	m_backPixelShader->Set();

	//マテリアルセット
	MATERIAL backMaterial = {};
	backMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f * m_alpha);
	backMaterial.textureEnable = false;
	RENDERER.SetMaterial(backMaterial);

	// 角丸矩形のプロパティ設定
	SHADER_PROPERTIES properties = {};
	properties.params1.x = 0.2f;	// 角丸半径
	properties.params1.y = 0.005f;	// 境界線のスムーズさ
	properties.params1.z = 1.0f;	// 矩形サイズ
	properties.params1.w = 1.0f;	// 矩形サイズ
	RENDERER.SetShaderProperties(properties);

	//スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);

	// ランク描画
	m_pixelShader->Set();

	// テクスチャセット
	int rank = m_player->GetRank() - 1;
	rank = std::max(0, rank);
	rank = std::min(MAX_RANKS - 1, rank);
	m_rankTextures[rank]->Set(0);

	//マテリアルセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, m_alpha);
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	//スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
