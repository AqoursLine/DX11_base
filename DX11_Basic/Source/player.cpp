#include "player.h"
#include "model.h"
#include "input.h"
#include "box.h"
#include "field.h"

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
	m_position = { 0.0f, 1.0f, 0.0f };

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

	//入力を車両制御に反映
	SetThrottle(m_smoothedInput.forward - m_smoothedInput.reverse);
	SetSteering(m_smoothedInput.steering);
	SetBrake(m_smoothedInput.brake);

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
	Vector3 velNorm = GetVelocity();
	velNorm.Normalize();

	arrowRot.y = std::atan2f(velNorm.x, velNorm.z);

	m_field->Draw(arrowPos, arrowRot, Vector3 { 1.0f, 1.0f, 1.0f });

	//デバッグ情報表示
	//std::cout << "Speed: " << GetSpeed() * 3.6f << " km/h" << std::endl;
	//std::cout << "RPM: " << GetRPM() << " rpm" << std::endl;
	//std::cout << "Position: (" << m_position.x << ", " << m_position.y << ", " << m_position.z << ")" << std::endl;
	//std::cout << "Rotation: (" << m_rotation.x << ", " << m_rotation.y << ", " << m_rotation.z << ")" << std::endl;

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

	float steerRate;
	steerRate = std::min(1.0f, m_steerSmoothRate * dt);

	//ステアリングを戻す時は速くする
	if (std::fabsf(m_currentInput.steering) < std::fabsf(m_smoothedInput.steering)) {
		steerRate *= 1.5f;
		steerRate = std::min(1.0f, steerRate);
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

void Player::DrawWheels() const {
	//ホイール描画
	//for (int i = 0; i < 4; i++) {
	//	Vector3 wheelPos = GetWheelPosition(i);
	//	float steerAngle = GetWheelSteerAngle(i);
	//	Vector3 wheelRot = Vector3::ZERO;
	//	wheelRot.x = m_wheelRotations[i];
	//	wheelRot.y = m_rotation.y + steerAngle;

	//	m_model->Draw(wheelPos, wheelRot, Vector3 { 0.5f, 1.0f, 1.0f });

	//}
}
