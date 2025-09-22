#include "tmp2D.h"
#include "sprite.h"

bool Temp2D::Initialize() {
	m_sprite = new Sprite();
	if (!m_sprite->Initialize(L"Asset\\Texture\\yukino.png")) {
		ErrorMessage(L"スプライトの初期化に失敗しました。", E_FAIL);
		return false;
	}

	m_position = {200.0f, 200.0f, 0.0f};
	m_rotation.z = XMConvertToRadians(45.0f);
	m_scale = { 459.0f, 600.0f, 1.0f };

	return true;
}

void Temp2D::Finalize() {
	m_sprite->Finalize();
	delete m_sprite;
}

void Temp2D::Update(double deltaTime) {

}

void Temp2D::Draw() const {
	m_sprite->Draw(m_position, m_rotation, m_scale);
}

void Temp2D::CleanUp() {

}