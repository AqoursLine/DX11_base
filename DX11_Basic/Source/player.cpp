#include "player.h"
#include "model.h"
#include "input.h"
#include "box.h"

bool Player::Initialize() {
	//m_model = new Model();
	//if (!m_model->LoadModelFBX("Asset\\Model\\testbox.fbx")) {
	//	return false;
	//}

	m_box = new Box();
	if (!m_box->Initialize()) {
		return false;
	}

	m_scale = {2.0, 0.6f, 4.0f};
	m_rotation = {0.0f, 0.0f, 0.0f};
	m_position = { 0.0f, 0.5f, 0.0f };

	if(!Vehicle::Initialize()) {
		return false;
	}

	return true;
}

void Player::Finalize() {
	if (m_model) {
		m_model->ReleaseModel();
		delete m_model;
	}
}

void Player::Update(double deltaTime) {
	UpdateInput(deltaTime);
	Vehicle::Update(deltaTime);
}

void Player::Draw() const {
//	m_model->Draw(m_position, m_rotation, m_scale);
	m_box->Draw(m_position, m_rotation, m_scale);
}

void Player::UpdateInput(double deltaTime) {
	if (Input::GetKeyPress(KK_W)) {
	}
}
