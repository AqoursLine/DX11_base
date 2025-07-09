#include "player.h"
#include "Component/model.h"
#include "System/input.h"

bool Player::Initialize() {
	m_model = new Model();
	if (!m_model->LoadModelFBX("Asset\\Model\\TheHerta2.fbx")) {
		return false;
	}

	m_scale = {1.0f, 1.0f, 1.0f};
	m_rotation = {0.0f, 0.0f, 0.0f};

	m_position.z = 2.0f;

	return true;
}

void Player::Finalize() {
	if (m_model) {
		m_model->ReleaseModel();
		delete m_model;
	}
}

void Player::Update(double deltaTime) {
	if (Input::GetKeyPress(KK_W)) {
		m_acceleration.z = m_moveSpeed;
	} else if (Input::GetKeyPress(KK_S)) {
		m_acceleration.z = -m_moveSpeed;
	} else {
		if (fabsf(m_velocity.z) < 0.5) {
			m_velocity.z = 0.0f;
			m_acceleration.z = 0.0f;
		} else {
			m_acceleration.z = m_velocity.z > 0 ? -m_moveSpeed : m_moveSpeed;
		}
	}

	//デバッグ用に上下
#ifdef _DEBUG
	if (Input::GetKeyPress(KK_SPACE)) {
		m_position.y += m_moveSpeed * static_cast<float>(deltaTime) * 0.1f;
	} else if (Input::GetKeyPress(KK_LEFTSHIFT)) {
		m_position.y -= m_moveSpeed * static_cast<float>(deltaTime) * 0.1f;
	}
#endif // _DEBUG


	if (Input::GetKeyPress(KK_A)) {
		m_rotation.y -= m_rotateSpeed * static_cast<float>(deltaTime);
	} else if (Input::GetKeyPress(KK_D)) {
		m_rotation.y += m_rotateSpeed * static_cast<float>(deltaTime);
	}

	Vector3 forward = GetForward();

	m_velocity += m_acceleration * static_cast<float>(deltaTime);
	if (m_velocity.z > 30.0f) {
		m_velocity.z = 30.0f; // 最大速度制限
	} else if (m_velocity.z < -30.0f) {
		m_velocity.z = -30.0f; // 最小速度制限
	}
	m_position += forward * m_velocity.z * static_cast<float>(deltaTime);
}

void Player::Draw() const {
	m_model->Draw(m_position, m_rotation, m_scale);
}
