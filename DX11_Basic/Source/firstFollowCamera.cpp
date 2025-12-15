#include "firstFollowCamera.h"

#include "main.h"

#include "racingBoat.h"
#include "scene.h"
#include "raceManager.h"

bool FirstFollowCamera::Initialize() {
	m_raceManager = m_scene->GetGameObject<RaceManager>();

	m_position = { 3.0f, 10.0f, -65.0f };
	m_targetPosition = { 0.0f, 0.0f, -10.0f };

	m_rate = 5.0f;

	m_isMainCamera = true;
	return true;
}

void FirstFollowCamera::Finalize() {
}

void FirstFollowCamera::Update(double deltaTime) {
	// レースマネージャーが存在しない場合は終了
	if (!m_raceManager) {
		return;
	}

	// 1位のボートを取得
	auto firstBoat = m_raceManager->GetRacingBoatByRank(1);

	// ボートが存在しない場合は終了
	if (!firstBoat) {
		return;
	}

	// ターゲット位置を1位のボートの位置に設定
	Vector3 finalTarget = firstBoat->GetPosition();

	// オフセットを加算
	finalTarget += Vector3(0.0f, 2.0f, 0.0f);

	// 補間係数
	float t = std::min(1.0f, static_cast<float>(deltaTime) * m_rate);

	// ターゲット位置へ線形補間で移動
	m_targetPosition = m_targetPosition + (finalTarget - m_targetPosition) * t;

	// カメラ位置を更新
	UpdatePosition(firstBoat, deltaTime);
}

void FirstFollowCamera::Draw() {

	Camera::Draw();
}

void FirstFollowCamera::CleanUp() {
}

void FirstFollowCamera::UpdatePosition(RacingBoat* targetBoat, double deltaTime) {
	// ボートの進捗度に応じてカメラの位置制御を調整
	if (!targetBoat) return;

	int lapCount = targetBoat->GetLapCount();
	if (lapCount <= 0) {
		// 最初の周回前は固定位置
		m_position = Vector3 { 5.0f, 10.0f, -65.0f };
	}

	float lapProgress = targetBoat->GetLapProgress();

	if ((lapProgress >= 0.15f && lapProgress < 0.35f) ||
		(lapProgress >= 0.65f && lapProgress < 0.85f)
		) {
		// 最初のカーブ中または2番目のカーブ中
		Vector3 boatVel = targetBoat->GetVelocity();
		boatVel.y = 0.0f;
		boatVel.Normalize();
		m_position = m_targetPosition + boatVel * 20.0f + Vector3(0.0f, 3.0f, 0.0f);
	} else if (lapProgress >= 0.35f && lapProgress < 0.65f) {
		// 北側の直線区間
		m_position = Vector3 { 0.0f, 10.0f, 0.0f };
	} else {
		// 南側の直線区間
		m_position = Vector3 { 5.0f, 10.0f, -65.0f };
	}
}
