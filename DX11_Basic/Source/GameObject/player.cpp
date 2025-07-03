#include "player.h"
#include "Component/model.h"

bool Player::Initialize() {
	m_model = new Model();
	if (!m_model->LoadModelFBX("Asset\\Model\\sportcar2.fbx")) {
		return false;
	}

	m_scale = {0.01f, 0.01f, 0.01f};
	m_rotation = {0.0f,0.0f, 0.0f};

	m_position.z = 2.0f;

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
