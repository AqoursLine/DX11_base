#include "titleMenuSelecter.h"
#include "scene.h"
#include "titleMenuIcon.h"

#include "input.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"


void TitleMenuSelecter::ClearMenuIcons() {
	for (auto& icon : m_menuIcons) {
		icon->SetActive(false);
		icon->IsSelected(false);
	}
	m_menuIcons.clear();
	m_currentIndex = 0;
	m_previousIndex = 0;
	m_maxIndex = 0;
}

bool TitleMenuSelecter::Initialize() {
	m_maxIndex = static_cast<int>(m_menuIcons.size());

	if (m_maxIndex == 0) {
		SetActive(false);
		return false;
	}

	// 最初のメニューアイコンを選択状態にする
	m_menuIcons[m_currentIndex]->IsSelected(true);

	// sprite関連初期化
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		return false;
	}
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset\\Texture\\start.png")) {
		return false;
	}
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\inversePS.cso");

	return true;
}

void TitleMenuSelecter::Finalize() {
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

void TitleMenuSelecter::Update(double deltaTime) {
	// 上下入力でメニュー移動
	if (Input::GetKeyTrigger(KK_W) || Input::GetKeyTrigger(KK_UP)) {
		m_previousIndex = m_currentIndex;
		m_currentIndex = (m_currentIndex - 1 + m_maxIndex) % m_maxIndex;
	}
	if (Input::GetKeyTrigger(KK_S) || Input::GetKeyTrigger(KK_DOWN)) {
		m_previousIndex = m_currentIndex;
		m_currentIndex = (m_currentIndex + 1) % m_maxIndex;
	}

	// 選択状態更新
	if (m_previousIndex != m_currentIndex) {
		m_menuIcons[m_previousIndex]->IsSelected(false);
		m_menuIcons[m_currentIndex]->IsSelected(true);
		m_previousIndex = m_currentIndex;
	}

	// 決定入力でメニュー決定
	if (Input::GetKeyTrigger(KK_ENTER) || Input::GetKeyTrigger(KK_SPACE)) {
		m_menuIcons[m_currentIndex]->OnDecide();
	}

}

void TitleMenuSelecter::Draw() {
	// 背景描画
	// シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	// テクスチャ設定
	m_texture->Set(0);

	// マテリアル
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = TRUE;
	RENDERER.SetMaterial(material);

	// アイコン位置・スケール取得
	Vector3 iconPosition = m_menuIcons[m_currentIndex]->GetPosition();

	float scaleFactor = 1.03f;

	Vector3 iconScale = m_menuIcons[m_currentIndex]->GetScale() * scaleFactor;

	// スプライト描画
	m_sprite->Draw(iconPosition, Vector3::ZERO, iconScale);

}
