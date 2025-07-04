#include "player.h"
#include "Component/model.h"
#include "System/input.h"

bool Player::Initialize() {
	m_model = new Model();
	if (!m_model->LoadModelFBX("Asset\\Model\\sportcar3.fbx")) {
		return false;
	}

	m_scale = {0.01f, 0.01f, 0.01f};
	m_rotation = {0.0f, 0.0f, 0.0f};

	m_position.z = 2.0f;

	return true;
}

void Player::Finalize() {
	if (m_model) {
		m_model->ReleaseModel();
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

	if (Input::GetKeyPress(KK_A)) {
		m_rotation.y -= m_rotateSpeed * static_cast<float>(deltaTime);
	} else if (Input::GetKeyPress(KK_D)) {
		m_rotation.y += m_rotateSpeed * static_cast<float>(deltaTime);
	}

	Vector3 forward = GetForward();

	m_velocity += m_acceleration * static_cast<float>(deltaTime);
	if (m_velocity.z > 30.0f) {
		m_velocity.z = 30.0f; // ç≈ëÂë¨ìxêßå¿
	} else if (m_velocity.z < -30.0f) {
		m_velocity.z = -30.0f; // ç≈è¨ë¨ìxêßå¿
	}
	m_position += forward * m_velocity.z * static_cast<float>(deltaTime);

	std::cout << "Player Position: " << m_position.x << ", " << m_position.y << ", " << m_position.z << std::endl;
	std::cout << "Player Velocity: " << m_velocity.x << ", " << m_velocity.y << ", " << m_velocity.z << std::endl;
	std::cout << "Player Acceleration: " << m_acceleration.x << ", " << m_acceleration.y << ", " << m_acceleration.z << std::endl;
	std::cout << "Player Rotation: " << m_rotation.x << ", " << m_rotation.y << ", " << m_rotation.z << std::endl;
}

void Player::Draw() const {
	m_model->Draw(m_position, m_rotation, m_scale);
}
