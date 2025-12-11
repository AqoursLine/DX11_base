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

	BoatResultData result = {};
	result.laneIndex = m_laneIndex;
	result.boatColor = m_boatColor;

	m_raceManager->SetResultData(result);
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

	// セクション判定
	UpdateProgressSection();

	// 進捗セクション更新
	CalculateLapProgress();

	Boat::Update(deltaTime);
}

void RacingBoat::Draw() {
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

void RacingBoat::UpdateProgressSection() {
	Vector3 pos = m_position;
	pos.y = 0.0f; // 水平成分のみ考慮

	if (pos.x >= 0.0f && pos.x < 150.0f && pos.z <= 0.0f) {
		// 南直線
		// 0.0f <= x < 150.0f, z <= 0.0f
		m_currentSection = FIRST_SOUTH_STRAIGHT;
	} else if (pos.x >= 150.0f && pos.z < 0.0f) {
		// 南東カーブ
		// x >= 150.0f, z < 0.0f
		m_currentSection = EAST_SOUTH_CURVE;
	} else if (pos.x > 150.0f && pos.z >= 0.0f) {
		// 北東カーブ
		// x > 150.0f, z >= 0.0f
		m_currentSection = EAST_NORTH_CURVE;
	} else if (pos.x > 0.0f && pos.x <= 150.0f && pos.z > 0.0f) {
		// 北直線
		// 0.0f < x <= 150.0f, z > 0.0f
		m_currentSection = FIRST_NORTH_STRAIGHT;
	} else if (pos.x > -150.0f && pos.x <= 0.0f && pos.z > 0.0f) {
		// 北直線
		// -150.0f < x <= 0.0f, z > 0.0f
		m_currentSection = SECOND_NORTH_STRAIGHT;
	} else if (pos.x <= -150.0f && pos.z > 0.0f) {
		// 北西カーブ
		// x < -150.0f, z > 0.0f
		m_currentSection = WEST_NORTH_CURVE;
	} else if (pos.x < -150.0f && pos.z <= 0.0f) {
		// 南西カーブ
		// x < -150.0f, z <= 0.0f
		m_currentSection = WEST_SOUTH_CURVE;
	} else {
		// 南直線
		m_currentSection = SECOND_SOUTH_STRAIGHT;
	}
}

void RacingBoat::CalculateLapProgress() {
	float progress = 0.0f;
	switch (m_currentSection) {
		case RacingBoat::FIRST_SOUTH_STRAIGHT:
			// 南直線
			// x = 0 ~ 150, z = -35
			progress = GetSectionProgress({ 150.0f, 0.0f, -35.0f }, { -1.0f, 0.0f, 0.0f });
			progress *= 0.2f;
			break;
		case RacingBoat::EAST_SOUTH_CURVE:
			// 南東カーブ
			// x = 150 ~ 185, z = -35 ~ 0
			progress = GetSectionProgress({ 150.0f, 0.0f, -35.0f }, { 1.0f, 0.0f, 0.0f }, { 185.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });
			progress = 0.2f + progress * 0.05f;
			break;
		case RacingBoat::EAST_NORTH_CURVE:
			// 北東カーブ
			// x = 185 ~ 150, z = 0 ~ 35
			progress = GetSectionProgress({ 185.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 150.0f, 0.0f, 35.0f }, { 1.0f, 0.0f, 0.0f });
			progress = 0.25f + progress * 0.05f;
			break;
		case RacingBoat::FIRST_NORTH_STRAIGHT:
			// 北直線
			// x = 150 ~ 0, z = 35
			progress = GetSectionProgress({ 0.0f, 0.0f, 35.0f }, { 1.0f, 0.0f, 0.0f });
			progress = 0.3f + progress * 0.2f;
			break;
		case RacingBoat::SECOND_NORTH_STRAIGHT:
			// 北直線
			// x = 0 ~ -150, z = 35
			progress = GetSectionProgress({ -150.0f, 0.0f, 35.0f }, { 1.0f, 0.0f, 0.0f });
			progress = 0.5f + progress * 0.2f;
			break;
		case RacingBoat::WEST_NORTH_CURVE:
			// 北西カーブ
			// x = -150 ~ -185, z = 35 ~ 0
			progress = GetSectionProgress({ -150.0f, 0.0f, 35.0f }, { -1.0f, 0.0f, 0.0f }, { -185.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
			progress = 0.7f + progress * 0.05f;
			break;
		case RacingBoat::WEST_SOUTH_CURVE:
			// 南西カーブ
			// x = -185 ~ -150, z = 0 ~ -35
			progress = GetSectionProgress({ -185.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { -150.0f, 0.0f, -35.0f }, { -1.0f, 0.0f, 0.0f });
			progress = 0.75f + progress * 0.05f;
			break;
		case RacingBoat::SECOND_SOUTH_STRAIGHT:
			progress = GetSectionProgress({ 0.0f, 0.0f, -35.0f }, { -1.0f, 0.0f, 0.0f });
			progress = 0.8f + progress * 0.2f;
			break;
		default:
			break;
	}

	m_lapProgress = progress;
}

/// <summary>
/// セクション進捗度取得(直線)
/// </summary>
/// <param name="pos">ゲートの位置</param>
/// <param name="dir">ゲートの正面方向</param>
/// <returns>セクション内の進捗度(0.0 ~ 1.0)</returns>
float RacingBoat::GetSectionProgress(const Vector3& pos, const Vector3& dir) {
	Vector3 toBoat = m_position - pos;
	toBoat.y = 0.0f; // 水平成分のみ考慮
	float sectionLength = toBoat.Dot(dir);

	return sectionLength / 150.0f;
}

/// <summary>
/// セクション進捗度取得(カーブ)
/// </summary>
/// <param name="pos1">ゲートの位置1</param>
/// <param name="dir1">ゲートの正面方向1</param>
/// <param name="pos2">ゲートの位置2</param>
/// <param name="dir2">ゲートの正面方向2</param>
/// <returns>セクション内の進捗度(0.0 ~ 1.0)</returns>
float RacingBoat::GetSectionProgress(const Vector3& pos1, const Vector3& dir1, const Vector3& pos2, const Vector3& dir2) {
	Vector3 toBoat1 = m_position - pos1;
	toBoat1.y = 0.0f; // 水平成分のみ考慮
	float sectionLength1 = toBoat1.Dot(dir1);

	Vector3 toBoat2 = m_position - pos2;
	toBoat2.y = 0.0f; // 水平成分のみ考慮
	float sectionLength2 = toBoat2.Dot(dir2);

	return (sectionLength1 * sectionLength2) / (60.0f * 60.0f);
}
