#include "multiSelector.h"

#include "renderer.h"
#include "texture.h"
#include "sprite.h"
#include "shaders.h"

#include "input.h"

bool MultiSelector::Initialize() {
	// スプライトの初期化
	m_sprite = new Sprite();
	if (!m_sprite->Initialize()) {
		return false;
	}

	// テクスチャの初期化
	m_texture = new Texture();
	if (!m_texture->Load(L"Asset/Texture/multiButtonStart.png")) {
		return false;
	}

	// シェーダーの初期化
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader/unlitTextureVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader/inversePS.cso");


	if (!m_buttons.empty()) {
		m_buttons[m_selectedIndex]->SetSelected(true);
	}

	return true;
}

void MultiSelector::Finalize() {
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

void MultiSelector::Update(double deltaTime) {
	// 入力処理
	// 右入力
	if (Input::GetKeyTrigger(KK_D) || Input::GetKeyTrigger(KK_RIGHT)) {
		m_buttons[m_selectedIndex]->SetSelected(false);

		m_selectedIndex++;
		if (m_selectedIndex >= m_buttons.size()) {
			m_selectedIndex = 0;
		}

		m_buttons[m_selectedIndex]->SetSelected(true);
	}

	// 左入力
	if (Input::GetKeyTrigger(KK_A) || Input::GetKeyTrigger(KK_LEFT)) {
		m_buttons[m_selectedIndex]->SetSelected(false);
		m_selectedIndex--;
		if (m_selectedIndex < 0) {
			m_selectedIndex = static_cast<int>(m_buttons.size()) - 1;
		}
		m_buttons[m_selectedIndex]->SetSelected(true);
	}

	// 決定入力
	if (Input::GetKeyTrigger(KK_ENTER)) {
		if (!m_buttons.empty()) {
			m_buttons[m_selectedIndex]->OnDecide();
		}
	}

}

void MultiSelector::Draw() {
	// シェーダーのセット
	m_vertexShader->Set();
	m_pixelShader->Set();

	// テクスチャのセット
	m_texture->Set(0);

	// マテリアルのセット
	MATERIAL material = {};
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.textureEnable = TRUE;
	RENDERER.SetMaterial(material);

	//　選択中のボタンに応じて場所とサイズを変更
	auto selectedButton = m_buttons[m_selectedIndex];
	m_position = selectedButton->GetPosition();
	m_scale = selectedButton->GetScale() * 1.03f; // 少し大きく表示


	// スプライトの描画
	m_sprite->Draw(m_position, m_rotation, m_scale);

}

