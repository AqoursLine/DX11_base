#include "player.h"
#include "input.h"
#include "field.h"

bool Player::Initialize() {
	if(!RacingBoat::Initialize()) {
		return false;
	}

	m_position = { -160.0f, 0.0f, -50.0f };

	SetIsMainPlayer(true);

	return true;
}

void Player::Update(double deltaTime) {
	UpdateInput(deltaTime);

	//入力を車両制御に反映
	SetThrottle(m_smoothedInput.throttle);
	SetSteering(m_smoothedInput.steering);
	SetBrake(m_smoothedInput.brake);

	RacingBoat::Update(deltaTime);
}

void Player::UpdateInput(double deltaTime) {
	//前進
	if (Input::GetKeyPress(KK_W)) {
		m_currentInput.throttle = 1.0f;
	} else if (Input::GetKeyPress(KK_S)) {
		m_currentInput.throttle = -1.0f;
	} else {
		m_currentInput.throttle = 0.0f;
	}

	//ステアリング
	if (Input::GetKeyPress(KK_A)) {
		m_currentInput.steering = -1.0f;
	} else if (Input::GetKeyPress(KK_D)) {
		m_currentInput.steering = 1.0f;
	} else {
		m_currentInput.steering = 0.0f;
	}

	//ブレーキ
	if (Input::GetKeyPress(KK_SPACE)) {
		m_currentInput.brake = 1.0f;
	} else {
		m_currentInput.brake = 0.0f;
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

	//ステアリング入力の平滑化
	m_smoothedInput.steering = std::lerp(m_smoothedInput.steering, m_currentInput.steering, steerRate);

	//推進入力の平滑化
	float throttleRate;
	throttleRate = std::min(1.0f, m_throttlSmoothRate * dt);
	m_smoothedInput.throttle = std::lerp(m_smoothedInput.throttle, m_currentInput.throttle, throttleRate);

	//ブレーキはそのまま
	m_smoothedInput.brake = m_currentInput.brake;
}

