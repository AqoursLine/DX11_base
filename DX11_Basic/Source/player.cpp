#include "player.h"
#include "model.h"
#include "input.h"

bool Player::Initialize() {
	m_model = new Model();
	if (!m_model->LoadModelFBX("Asset\\Model\\sportcar3.fbx")) {
		return false;
	}

	m_scale = {0.01f, 0.01f, 0.01f};
	m_rotation = {0.0f, 0.0f, 0.0f};
	m_position = { 0.0f, 0.5f, 0.0f };

	m_maxSpeed = 150.0f; // 最大速度を120km/hに設定
	m_acceleration = 120.0f;
	m_deceleration = m_acceleration * 0.8f; // 減速度を120km/hに設定

	m_sideMaxForce = m_maxSpeed / 5.0f; // 横移動の最大速度を最大速度に設定
	m_sideAcceleration = m_sideMaxForce * 2.0f; // 横移動の加速度を120km/hに設定

	return true;
}

void Player::Finalize() {
	if (m_model) {
		m_model->ReleaseModel();
		delete m_model;
	}
}

void Player::Update(double deltaTime) {
	Vector3 velocity = { 0.0f, 0.0f, 0.0f };

	bool isWPressed = Input::GetKeyPress(KK_W);
	bool isSPressed = Input::GetKeyPress(KK_S);

	// 前進キーが押されている場合、加速する
	float acc = 0.0f;
	if (isWPressed && !isSPressed) {
		acc = m_acceleration * static_cast<float>(deltaTime);
	} else if (!isWPressed && isSPressed) {
		// 後退キーが押されている場合、減速する
		acc = -m_acceleration * static_cast<float>(deltaTime);
	} else {
		// どちらのキーも押されていない場合、減速する
		if (m_moveSpeed > 0.0f) {
			if (m_moveSpeed <= m_deceleration) {
				m_moveSpeed = 0.0f; // 減速が移動速度を下回る場合は停止
			} else {
				acc = -m_deceleration * static_cast<float>(deltaTime); // 減速を適用
			}
		} else if (m_moveSpeed < 0.0f) {
			if (m_moveSpeed >= -m_deceleration) {
				m_moveSpeed = 0.0f; // 減速が移動速度を下回る場合は停止
			} else {
				acc = m_deceleration * static_cast<float>(deltaTime); // 減速を適用
			}
		} else {
			acc = 0.0f; // 移動速度が0の場合は加速しない
		}
	}

	//移動速度を更新
	m_moveSpeed += acc;
	//最大速度を超えないように制限
	m_moveSpeed = XMMax(-m_maxSpeed, XMMin(m_moveSpeed, m_maxSpeed));

	//横方向の移動
	bool isDPressed = Input::GetKeyPress(KK_D);
	bool isAPressed = Input::GetKeyPress(KK_A);

	float maxSideSpeed = XMMin(m_sideMaxForce, fabsf(m_moveSpeed / 5.0f));

	if (isDPressed && !isAPressed) {
		m_sideForce += m_sideAcceleration * static_cast<float>(deltaTime);
		if (m_sideForce > maxSideSpeed) {
			m_sideForce = maxSideSpeed;
		}
	} else if (!isDPressed && isAPressed) {
		m_sideForce -= m_sideAcceleration * static_cast<float>(deltaTime);
		if (m_sideForce < -maxSideSpeed) {
			m_sideForce = -maxSideSpeed;
		}
	} else {
		if (m_sideForce > 0.0f) {
			m_sideForce -= m_sideAcceleration * static_cast<float>(deltaTime);
			if (m_sideForce < 0.0f) m_sideForce = 0.0f;
		} else if (m_sideForce < 0.0f) {
			m_sideForce += m_sideAcceleration * static_cast<float>(deltaTime);
			if (m_sideForce > 0.0f) m_sideForce = 0.0f;
		}

		//上限を超えないように補正
		if (m_sideForce > maxSideSpeed) {
			m_sideForce = maxSideSpeed;
		}
		if (m_sideForce < -maxSideSpeed) {
			m_sideForce = -maxSideSpeed;
		}
	}

	//速度が0のときは横方向の力をリセット
	if (m_moveSpeed == 0.0f) {
		m_sideForce = 0.0f;
	}

	Vector3 forward = GetForward();

	//ベロシティを設定
	velocity = forward * m_moveSpeed * static_cast<float>(deltaTime);

	//横方向のベロシティを設定
	velocity += GetRight() * m_sideForce * static_cast<float>(deltaTime);

	//位置を更新
	if (fabsf(m_moveSpeed)) {
		m_position += velocity;
	}

	//回転処理
	Vector3 rotationVector = forward * fabsf(m_moveSpeed) * static_cast<float>(deltaTime);
	rotationVector += GetRight() * (m_moveSpeed >= 0 ? 1.0f : -1.0f) * m_sideForce * static_cast<float>(deltaTime);
	// 回転量を計算
	float rotationAngle = atan2f(rotationVector.x, rotationVector.z);
	m_rotation.y = rotationAngle; // Y軸回転を設定


	//consoleをクリア
	std::cout << "\033[H";
	//デバッグ用の情報を表示
	std::cout << "Move Speed: " << m_moveSpeed << ", Side Speed:" << m_sideForce << std::endl;
	std::cout << "Position: (" << m_position.x << ", " << m_position.y << ", " << m_position.z << ")" << std::endl;
	std::cout << "Rotation: (" << m_rotation.x << ", " << m_rotation.y << ", " << m_rotation.z << ")" << std::endl;
}

void Player::Draw() const {
	m_model->Draw(m_position, m_rotation, m_scale);
}
