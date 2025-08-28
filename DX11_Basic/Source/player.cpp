#include "player.h"
#include "model.h"
#include "input.h"
#include "box.h"
#include <algorithm>

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

	//エンジン力計算
	float engineForce = 0.0f;
	if (m_smoothedInput.forward > 0.01f) {
		//前進
		engineForce = m_smoothedInput.forward * m_params.maxEngineForce;
		m_isReversing = false;
	}else if (m_smoothedInput.reverse > 0.01f) {
		//後退
		engineForce = -m_smoothedInput.reverse * m_params.maxEngineForce * m_reverseForceRatio;
		m_isReversing = true;
	} else {
		//ニュートラル
		engineForce = 0.0f;

		//現在の速度が小さい場合は後退フラグを解除
		if (std::abs(GetCurrentSpeed()) < 1.0f) {
			m_isReversing = false;
		}
	}

	//エンジン力を適用
	SetEngineForce(engineForce);

	//ステアリングを適用
	SetSteeringValue(m_smoothedInput.steering * m_params.maxSteeringAngle);

	//ブレーキ力を適用
	float brakeForce = m_smoothedInput.brake * m_params.maxBrakingForce;

	//サイドブレーキ
	if (m_smoothedInput.handbrake) {
		brakeForce = m_params.maxBrakingForce;
	}

	SetBrakingForce(brakeForce);

	Vehicle::Update(deltaTime);
}

void Player::Draw() const {
//	m_model->Draw(m_position, m_rotation, m_scale);
	m_box->Draw(m_position, m_rotation, m_scale);
}

void Player::UpdateInput(double deltaTime) {
	m_currentInput.forward = Input::GetKeyPress(KK_W) ? 1.0f : 0.0f;
	m_currentInput.reverse = Input::GetKeyPress(KK_S) ? 1.0f : 0.0f;
	m_currentInput.brake = Input::GetKeyPress(KK_SPACE) ? 1.0f : 0.0f;
	m_currentInput.steering = (Input::GetKeyPress(KK_D) ? 1.0f : 0.0f) - (Input::GetKeyPress(KK_A) ? 1.0f : 0.0f);
	m_currentInput.handbrake = Input::GetKeyPress(KK_LEFTSHIFT);

	SmoothInput(deltaTime);
}

void Player::SmoothInput(double deltaTime) {
	float dt = static_cast<float>(deltaTime);

	//前進入力の平滑化
	float forwardRate = m_forwardSmoothRate * dt;
	m_smoothedInput.forward = std::lerp(m_smoothedInput.forward, m_currentInput.forward, forwardRate);

	//後退入力の平滑化(後退は前進よりもゆっくり反応する)
	float reverseRate = m_reverseSmoothRate * dt;
	m_smoothedInput.reverse = std::lerp(m_smoothedInput.reverse, m_currentInput.reverse, reverseRate);

	//ブレーキ入力の平滑化(ブレーキは素早く反応する)
	float brakeRate = m_brakeSmoothRate * dt;
	m_smoothedInput.brake = std::lerp(m_smoothedInput.brake, m_currentInput.brake, brakeRate);

	//ステアリング入力の平滑化
	float steerRate = m_steerSmoothRate * dt;

	//ステアリングを戻す時はより早く戻る
	if (std::abs(m_currentInput.steering) < std::abs(m_smoothedInput.steering)) {
		steerRate *= 1.5f;
	}

	m_smoothedInput.steering = std::lerp(m_smoothedInput.steering, m_currentInput.steering, steerRate);

	//ハンドブレーキは即座に反映
	m_smoothedInput.handbrake = m_currentInput.handbrake;
}

float Player::CalculateRPM() const {
	//現在の速度を取得
	float speed = std::abs(GetCurrentSpeed());
	float maxSpeed = GetMaxSpeed();

	//速度比からRPMを計算
	float speedRatio = speed / maxSpeed;
	speedRatio = std::clamp(speedRatio, 0.0f, 1.0f);

	//アクセル入力も考慮
	float throttle = max(max( m_smoothedInput.forward, m_smoothedInput.reverse), 0.1f );

	float rpm = m_idleRPM + (m_maxRPM - m_idleRPM) * speedRatio * throttle;

	return rpm;
}
