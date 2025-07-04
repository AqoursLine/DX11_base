#include "ball.h"
#include "Component/model.h"
#include "System/system.h"
#include "System/physics.h"
#include "System/input.h"

bool Ball::Initialize() {
	m_model = new Model();
	if (!m_model->LoadModelFBX("Asset\\Model\\ball.fbx")) {
		return false;
	}

	m_position.y = 5.0f; // 初期位置を設定

	m_body = SYSTEM.GetPhysics()->CreateDynamicSphere(
		m_position,
		m_rotation,
		0.5f, // 半径
		1.0f  // 質量
	);

    return true;
}

void Ball::Finalize() {
	if (m_model) {
		m_model->ReleaseModel();
		m_model = nullptr;
	}
}

void Ball::Update(double deltaTime) {
	if (m_body) {
		Vector3 position, rotation;
		SYSTEM.GetPhysics()->GetObjectTransform(m_body, position, rotation);
		m_position = position;
		m_rotation = rotation;

		if (Input::GetKeyTrigger(KK_ENTER)) {
			m_body->applyCentralImpulse(btVector3(0.0f, 20, 0.0f)); // 重力を適用
		}
	}
}

void Ball::Draw() const {
	if (m_model) {
		m_model->Draw(m_position, m_rotation, m_scale);
	}
}
