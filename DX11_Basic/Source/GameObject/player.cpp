#include "player.h"
#include "Component/model.h"

bool Player::Initialize() {
	m_model = new Model();
	if (!m_model->LoadModelFBX("Asset\\Model\\TheHerta.fbx")) {
		return false;
	}

	m_scale = {1.0f, 1.0f, 1.0f};

	return true;
}

void Player::Finalize() {
	if (m_model) {
		m_model->ReleaseModel();
	}
}

void Player::Update(double deltaTime) {
}

void Player::Draw() const {
	m_model->Draw(m_position, m_rotation, m_scale);
}
