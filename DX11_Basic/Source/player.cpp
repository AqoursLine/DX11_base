#include "player.h"
#include "model.h"
#include "input.h"
#include "box.h"
#include "field.h"
#include <algorithm>

bool Player::Initialize() {
	m_model = new Model();
	if (!m_model->LoadModelFBX("Asset\\Model\\tire.fbx")) {
		return false;
	}

	m_box = new Box();
	if (!m_box->Initialize()) {
		return false;
	}

	m_field = new Field();
	if (!m_field->Initialize(L"Asset\\Texture\\arrow.png")) {
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
	if (m_box) {
		m_box->Finalize();
		delete m_box;
	}
	if (m_field) {
		m_field->Finalize();
		delete m_field;
	}

	Vehicle::Finalize();
}

void Player::Update(double deltaTime) {
	UpdateInput(deltaTime);

	//停止状態管理
	UpdateStationaryState();

	//エンジン力計算
	float engineForce = 0.0f;
	if (m_smoothedInput.forward > 0.01f) {
		//前進
		engineForce = m_smoothedInput.forward * m_params.maxEngineForce;
	}else if (m_smoothedInput.reverse > 0.01f) {
		//後退
		engineForce = -m_smoothedInput.reverse * m_params.maxEngineForce;
	} else {
		//ニュートラル
		engineForce = 0.0f;
	}

	//エンジン力を適用
	SetEngineForce(engineForce);

	//ステアリングを適用
	SetSteeringValue(m_smoothedInput.steering * m_params.maxSteeringAngle);

	//ブレーキ力を適用
	float brakeForce = m_smoothedInput.brake * m_params.maxBrakingForce;
	SetBrakingForce(brakeForce);

	//サイドブレーキ
	SetHandbrake(m_smoothedInput.handbrake);


	Vehicle::Update(deltaTime);
}

void Player::Draw() const {
//	m_model->Draw(m_position, m_rotation, m_scale);
	m_box->Draw(m_position, m_rotation, m_scale);

	DrawWheels();

	//進行方向描画
	Vector3 arrowPos = m_position + Vector3 { 0.0f, 1.0f, 0.0f };

	//velocityから進行方向を計算
	Vector3 arrowRot = Vector3::ZERO;
	Vector3 velNorm = m_velocity;
	velNorm.Normalize();

	arrowRot.y = std::atan2f(velNorm.x, velNorm.z);

	m_field->Draw(arrowPos, arrowRot, Vector3 { 1.0f, 1.0f, 1.0f });

	//デバッグ表示
	std::cout << "Speed: " << GetCurrentSpeed() << " km/h" << std::endl;
	std::cout << "Engine Force: " << m_currentEngineForce << " N" << std::endl;
	std::cout << "Current State: " << static_cast<int>(m_vehicleState) << std::endl;
}

void Player::UpdateInput(double deltaTime) {
	//前進
	if (Input::GetKeyPress(KK_W)) {
		m_currentInput.forward = 1.0f;
	} else {
		m_currentInput.forward = 0.0f;
	}

	//後退
	if (Input::GetKeyPress(KK_S)) {
		m_currentInput.reverse = 1.0f;
	} else {
		m_currentInput.reverse = 0.0f;
	}

	//ブレーキ
	if (Input::GetKeyPress(KK_SPACE)) {
		m_currentInput.brake = 1.0f;
	} else {
		m_currentInput.brake = 0.0f;
	}

	//ステアリング
	if (Input::GetKeyPress(KK_A)) {
		m_currentInput.steering = -1.0f;
	} else if (Input::GetKeyPress(KK_D)) {
		m_currentInput.steering = 1.0f;
	} else {
		m_currentInput.steering = 0.0f;
	}

	//ハンドブレーキ
	if (Input::GetKeyPress(KK_LEFTSHIFT)) {
		m_currentInput.handbrake = true;
	} else {
		m_currentInput.handbrake = false;
	}

	SmoothInput(deltaTime);
}

void Player::SmoothInput(double deltaTime) {
	float dt = static_cast<float>(deltaTime);

	//停止中はステアリングの平滑化を速くする
	float steerRate;
	if (m_isStationary) {
		steerRate = std::min(1.0f, m_steerSmoothRate * 2.0f * dt);
	} else {
		steerRate = std::min(1.0f, m_steerSmoothRate * dt);

		//ステアリングを戻す時は速くする
		if (std::fabsf(m_currentInput.steering) < std::fabsf(m_smoothedInput.steering)) {
			steerRate *= 1.5f;
			steerRate = std::min(1.0f, steerRate);
		}
	}

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
	float throttle = std::max(std::max(m_smoothedInput.forward, m_smoothedInput.reverse), 0.1f );

	float rpm = m_idleRPM + (m_maxRPM - m_idleRPM) * speedRatio * throttle;

	return rpm;
}

void Player::DrawWheels() const {
	//ホイール描画
	for (int i = 0; i < 4; i++) {
		Vector3 wheelPos = GetWheelPosition(i);
		Vector3 wheelRot = GetWheelRotation(i);
		Vector3 wheelScale = { 0.5f, 1.0f, 1.0f };
		m_model->Draw(wheelPos, wheelRot, wheelScale);
	}
}

void Player::UpdateStationaryState() {
	m_isStationary = (std::abs(GetCurrentSpeed()) < 1.8f);
}
