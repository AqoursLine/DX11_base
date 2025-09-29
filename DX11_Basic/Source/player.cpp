#include "player.h"
#include "model.h"
#include "input.h"
#include "box.h"
#include "field.h"

#include "system.h"
#include "webClient.h"

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
	m_scale = { 2.5f, 1.0f, 4.0f };
	m_rotation = {0.0f, 0.0f, 0.0f};
	m_position = { 0.0f, 0.0f, 0.0f };
	m_quaternion = Vector4::IDENTITY;

	if(!Boat::Initialize()) {
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

	Boat::Finalize();
}

void Player::Update(double deltaTime) {
	UpdateInput(deltaTime);

	//入力を車両制御に反映
	SetThrottle(m_smoothedInput.throttle);
	SetSteering(m_smoothedInput.steering);

	Boat::Update(deltaTime);

	//ウェブにデータを送信
	auto webClient = SYSTEM.GetWebClient();
	if (webClient && webClient->IsConnected()) {
		json message;
		message["type"] = "position";
		message["x"] = m_position.x;
		message["y"] = m_position.y;
		message["z"] = m_position.z;
		webClient->SendMessageClient(message);

		message["type"] = "rotation";
		message["x"] = m_quaternion.x;
		message["y"] = m_quaternion.y;
		message["z"] = m_quaternion.z;
		message["w"] = m_quaternion.w;
		webClient->SendMessageClient(message);

		message["type"] = "speed";
		message["speed"] = GetSpeedKmh(); // km/h
		webClient->SendMessageClient(message);
	}
}

void Player::Draw() const {
//	m_model->Draw(m_position, m_rotation, m_scale);
	m_box->Draw(m_position, m_quaternion, m_scale);

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
	std::cout << "Speed: " << GetSpeed() * 3.6f << " km/h" << std::endl;
	std::cout << "Position: (" << m_position.x << ", " << m_position.y << ", " << m_position.z << ")" << std::endl;
	Vector3 vel = GetVelocity();
	std::cout << "Velocity: (" << vel.x << ", " << vel.y << ", " << vel.z << ")" << std::endl;
	Vector3 acc = GetAcceleration();
	std::cout << "Acceleration: (" << acc.x << ", " << acc.y << ", " << acc.z << ")" << std::endl;
	std::cout << "Rotation: (" << m_rotation.x << ", " << m_rotation.y << ", " << m_rotation.z << ")" << std::endl;
	std::cout << "Quaternion: (" << m_quaternion.x << ", " << m_quaternion.y << ", " << m_quaternion.z << ", " << m_quaternion.w << ")" << std::endl;
	std::cout << "Throttle Input: " << m_smoothedInput.throttle << std::endl;

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
