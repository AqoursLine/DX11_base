#include "racingBoat.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "raceManager.h"

#include "webClient.h"

#include "renderer.h"
#include "Shaders.h"

#include "modelRenderer.h"

#ifdef _DEBUG
#include "imguiSystem.h"
#endif // _DEBUG



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
	result.playerName = m_racerName;

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

#ifdef _DEBUG
	//デバッグ情報表示
	std::string title = "RacingBoat Debug Info (Lane " + std::to_string(m_laneIndex) + ")";
	ImGui::Begin(title.c_str());
	ImGui::Text("Lap Count: %d", m_lapCount);
	ImGui::Text("Lap Progress: %.2f%%", m_lapProgress * 100.0f);
	ImGui::Text("Current Section: %d", static_cast<int>(m_currentSection));
	ImGui::Text("Rank: %d", m_rank);
	ImGui::End();

#endif // _DEBUG

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

	if (pos.z < 0 && pos.x >= 0 && pos.x < 150.0f) {
		// 最初の南直線
		m_currentSection = FIRST_SOUTH_STRAIGHT;
	} else if (pos.z >= 0 && pos.x <= 150.0f && pos.x > 0) {
		// 前半の北直線
		m_currentSection = FIRST_NORTH_STRAIGHT;
	} else if (pos.z >= 0 && pos.x <= 0 && pos.x > -150.0f) {
		// 後半の北直線
		m_currentSection = SECOND_NORTH_STRAIGHT;
	} else if (pos.z < 0 && pos.x >= -150.0f && pos.x < 0) {
		// 最後の南直線
		m_currentSection = SECOND_SOUTH_STRAIGHT;
	} else if (pos.x >= 150.0f) {
		// 東カーブ
		m_currentSection = EAST_CURVE;
	} else if (pos.x <= -150.0f) {
		// 西カーブ
		m_currentSection = WEST_CURVE;
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
		case RacingBoat::EAST_CURVE:
			// 東カーブ
			progress = GetSectionProgressCurve({ 150.0f, 0.0f, 0.0f }, -XM_PIDIV2);
			progress = 0.2f + progress * 0.1f;
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
		case RacingBoat::WEST_CURVE:
			// 西カーブ
			// オフセットを-πにして、x <= -150より左側の範囲を0~πに収める
			progress = GetSectionProgressCurve({ -150.0f, 0.0f, 0.0f }, XM_PIDIV2);
			progress = 0.7f + progress * 0.1f;
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

	return 1.0f - (sectionLength / 150.0f);
}

/// <summary>
/// セクション進捗度取得(カーブ)
/// </summary>
/// <param name="center">カーブ中心位置</param>
/// <param name="offset">カーブの開始角度</param>
/// <param name="totalAngle">カーブ全体の角度</param>
/// <returns>カーブ内の進捗度(0.0f ~ 1.0f)</returns>
float RacingBoat::GetSectionProgressCurve(const Vector3& center, float offset, float totalAngle) {
	Vector3 toBoat = m_position - center;

	// ボートへのベクトルの角度を計算
	float angle = std::atan2(toBoat.z, toBoat.x);

	// 角度を0~2πの範囲に正規化
	if (angle < 0) {
		angle += XM_2PI;
	}
	if (offset < 0) {
		offset += XM_2PI;
	}

	// カーブ内の進捗度を計算
	float angleDelta = angle - offset;

	// 角度が負の場合、全体角度を加算
	if (angleDelta < 0) {
		angleDelta += XM_2PI;
	}

	return angleDelta / totalAngle;
}

