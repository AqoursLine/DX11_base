#include "main.h"
#include "rankDisplay.h"
#include "sprite.h"
#include "renderer.h"
#include "texture.h"
#include "shaders.h"

#include "scene.h"
#include "player.h"

bool RankDisplay::Initialize() {
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\spriteAnimationVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\spriteAnimationPS.cso");

	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\dotNum.png")) {
		ErrorMessage(L"順位表示のテクスチャの読み込みに失敗しました。", E_FAIL);
		return false;
	}
	m_sprite = new Sprite();

	m_scale = { 60.f, 112.0f, 1.0f };
	m_position = { 20.0f + m_scale.x * 0.5f, SCREEN_HEIGHT - m_scale.y * 0.5f - 20.0f, 0.0f };

	m_player = m_scene->GetGameObject<Player>();

	if (!m_player) {
		SetActive(false);
	}

	return true;
}

void RankDisplay::Finalize() {
	if (m_sprite) {
		m_sprite->Finalize();
		delete m_sprite;
	}
	if (m_texture) {
		delete m_texture;
	}
	if (m_vertexShader) {
		delete m_vertexShader;
	}
	if (m_pixelShader) {
		delete m_pixelShader;
	}
}

void RankDisplay::Update(double deltaTime) {
}

void RankDisplay::Draw() {
	//シェーダーの設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	//テクスチャの設定
	m_texture->Set(0);

	//マテリアルセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 0.9f, 0.2f, 1.0f);
	material.textureEnable = true;
	RENDERER.SetMaterial(material);

	//順位取得
	int rank = m_player->GetRank();

	//シェーダープロパティ設定
	SHADER_PROPERTIES properties = {};
	properties.params1.z = 1.0f / 10.0f; //1桁あたりの幅
	properties.params1.w = 0.5f; //1桁あたりの高さ
	properties.params1.x = static_cast<float>(rank) * properties.params1.z; //フレームのX位置
	properties.params1.y = 0.0f;
	RENDERER.SetShaderProperties(properties);

	//スプライト描画
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
