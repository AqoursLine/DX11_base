#include "fpsCamera.h"
#include "input.h"

void FpsCamera::Update(double deltaTime) {
	if (Input::GetKeyPress(KK_W)) {
		MoveForward(true, deltaTime);
	}
	if (Input::GetKeyPress(KK_S)) {
		MoveForward(false, deltaTime);
	}
	if (Input::GetKeyPress(KK_A)) {
		MoveSide(false, deltaTime);
	}
	if (Input::GetKeyPress(KK_D)) {
		MoveSide(true, deltaTime);
	}
	if (Input::GetKeyPress(KK_SPACE)) {
		MoveUpDown(true, deltaTime);
	}
	if (Input::GetKeyPress(KK_LEFTSHIFT)) {
		MoveUpDown(false, deltaTime);
	}

	//回転
	if (Input::GetKeyPress(KK_RIGHT)) {
		m_rotation.y += m_rotateSpeed * static_cast<float>(deltaTime);
	}
	if (Input::GetKeyPress(KK_LEFT)) {
		m_rotation.y -= m_rotateSpeed * static_cast<float>(deltaTime);
	}
	if (Input::GetKeyPress(KK_DOWN)) {
		m_rotation.x += m_rotateSpeed * static_cast<float>(deltaTime);
		if (m_rotation.x > XMConvertToRadians(89.0f)) {
			m_rotation.x = XMConvertToRadians(89.0f);
		}
	}
	if (Input::GetKeyPress(KK_UP)) {
		m_rotation.x -= m_rotateSpeed * static_cast<float>(deltaTime);
		if (m_rotation.x < XMConvertToRadians(-89.0f)) {
			m_rotation.x = XMConvertToRadians(-89.0f);
		}
	}

	//ターゲットの位置を回転
	RotattionCamera(deltaTime);
}

void FpsCamera::RotattionCamera(double deltaTime) {
	// カメラのオフセット位置を計算
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	//オフセットベクトル(単位ベクトル)
	XMVECTOR offsetVector = XMVECTOR { 0.0f, 0.0f, 1.0f, 0.0f };

	offsetVector = XMVector3Transform(offsetVector, rotationMatrix);
	//ターゲット位置をカメラの位置からオフセットを足した位置に設定
	m_targetPosition = m_position + Vector3(
		XMVectorGetX(offsetVector),
		XMVectorGetY(offsetVector),
		XMVectorGetZ(offsetVector)
	);
}
