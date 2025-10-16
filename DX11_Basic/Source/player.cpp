#include "player.h"
#include "input.h"
#include "field.h"

#include "system.h"
#include "webClient.h"

#include "renderer.h"
#include "texture.h"
#include "Shaders.h"

#include "modelRenderer.h"

bool Player::Initialize() {

	//モデルロード
	m_model = new ModelRenderer();
	if (!m_model->Load("Asset\\Model\\boat.fbx")) {
		ErrorMessage(L"モデルの読み込みに失敗しました。", E_FAIL);
		return false;
	}

	//環境光設定
	m_model->SetMaterialAmbientColor(0, Vector4 { 0.5f, 0.5f, 0.5f, 1.0f });

	//シェーダーロード
	m_vertexShader = new VertexShader();
	m_vertexShader->Load(L"Shader\\pixelLightingVS.cso");
	m_pixelShader = new PixelShader();
	m_pixelShader->Load(L"Shader\\pixelLightingPS.cso");

	m_scale = { 100.0f, 100.0f, 100.0f };

	if(!RacingBoat::Initialize()) {
		return false;
	}

	return true;
}

void Player::Finalize() {
	if (m_model) {
		delete m_model;
		m_model = nullptr;
	}

	delete m_vertexShader;
	m_vertexShader = nullptr;
	delete m_pixelShader;
	m_pixelShader = nullptr;

	Boat::Finalize();
}

void Player::Update(double deltaTime) {
	UpdateInput(deltaTime);

	//入力を車両制御に反映
	SetThrottle(m_smoothedInput.throttle);
	SetSteering(m_smoothedInput.steering);
	SetBrake(m_smoothedInput.brake);

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

	//シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	//マテリアルセット
	m_model->SetMaterialDiffuseColor(1, Vector4 { 0.2f, 0.6f, 1.0f, 1.0f });

	//モデル描画
	m_model->Draw(m_position, m_quaternion, m_scale);
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

