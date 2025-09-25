#include "titleText.h"
#include "sprite.h"

bool TitleText::Initialize() {
	m_sprite = new Sprite();
	if (!m_sprite->Initialize(L"Asset\\Texture\\title.jpg")) {
		ErrorMessage(L"タイトルテキストのスプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}
	m_position = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0.0f };
	m_scale = { 800.0f, 200.0f, 1.0f };
	return true;

}

void TitleText::Finalize() {
	m_sprite->Finalize();
	delete m_sprite;
}

void TitleText::Update(double deltaTime) {
}

void TitleText::Draw() const {
	m_sprite->Draw(m_position, m_rotation, m_scale);
}
