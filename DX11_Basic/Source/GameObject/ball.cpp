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

	m_position.y = 5.0f; // ‰ŠúˆÊ’u‚ðÝ’è

    return true;
}

void Ball::Finalize() {
	if (m_model) {
		m_model->ReleaseModel();
		delete m_model;
		m_model = nullptr;
	}
}

void Ball::Update(double deltaTime) {
}

void Ball::Draw() const {
	if (m_model) {
		m_model->Draw(m_position, m_rotation, m_scale);
	}
}
