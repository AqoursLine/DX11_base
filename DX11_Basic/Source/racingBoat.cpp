#include "racingBoat.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "raceManager.h"

#include "webClient.h"

#include "renderer.h"
#include "Shaders.h"

#include "modelRenderer.h"


void RacingBoat::SetThrottle(float throttle) {
	float adjustedThrottle = throttle;

	if (m_isStarted && !m_isPassedGoalGate) {
		//スタート開始後
		if (throttle < 0) {
			//リバースギア
			SetReverse(true);
		} else {
			SetReverse(false);
		}

		throttle = std::abs(throttle);
	} else {
		throttle = 0.0f;
	}

	Boat::SetThrottle(throttle);
}

void RacingBoat::SetLaneIndex(int index) {
	m_laneIndex = index;

	//レーンに応じたカラー設定
	switch (m_laneIndex) {
		case 0:
			//レーン0: 白
			m_boatColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			break;
		case 1:
			//レーン1: 黒
			m_boatColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
			break;
		case 2:
			//レーン2: 赤
			m_boatColor = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
			break;
		case 3:
			//レーン3: 青
			m_boatColor = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
			break;
		case 4:
			//レーン4: 黄
			m_boatColor = Vector4(1.0f, 1.0f, 0.0f, 1.0f);
			break;
		case 5:
			//レーン5: 緑
			m_boatColor = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
			break;
		default:
			//その他: 灰色
			m_boatColor = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
			break;

	}
}

void RacingBoat::FinishRace() {
	//レース終了処理
	m_raceManager->SetLaneTime(m_laneIndex);
}

bool RacingBoat::Initialize() {
	m_rotation = { 0.0f, XM_PIDIV2, 0.0f };

	//ボートの初期方向をセット
	SetStartYaw(m_rotation.y);

	//レースマネージャ取得
	m_raceManager = m_scene->GetGameObject<RaceManager>();

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

	return Boat::Initialize();
}

void RacingBoat::Finalize() {
	if (m_model) {
		delete m_model;
		m_model = nullptr;
	}

	delete m_vertexShader;
	m_vertexShader = nullptr;
	delete m_pixelShader;
	m_pixelShader = nullptr;
}

void RacingBoat::Update(double deltaTime) {
	//ウェブにデータを送信
	auto webClient = SYSTEM.GetWebClient();
	if (webClient && webClient->IsConnected()) {
		json message;
		message["ID"] = m_laneIndex;

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

	Boat::Update(deltaTime);
}

void RacingBoat::Draw() const {
	//シェーダー設定
	m_vertexShader->Set();
	m_pixelShader->Set();

	//マテリアルセット
	m_model->SetMaterialDiffuseColor(1, m_boatColor);

	//モデル描画
	m_model->Draw(m_position, m_quaternion, m_scale);
}

Vector2 RacingBoat::GetSceneBoundsMin() const {
	return m_raceManager->GetBoundsMin();
}

Vector2 RacingBoat::GetSceneBoundsMax() const {
	return m_raceManager->GetBoundsMax();
}
