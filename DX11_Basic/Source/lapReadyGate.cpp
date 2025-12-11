#include "lapReadyGate.h"

#include "renderer.h"
#include "sprite.h"
#include "texture.h"
#include "shaders.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "raceManager.h"

#include "racingBoat.h"

bool LapReadyGate::Initialize() {
	//位置、回転、拡大縮小の設定
	m_scale = { 60.0f, 20.0f, 1.0f };
	m_position = { 0.0f, 10.0f, 30.0f };
	m_rotation = { 0.0f, XM_PIDIV2, XM_PI };

	//レースマネージャーの取得
	m_raceManager = m_scene->GetGameObject<RaceManager>();

	m_isPassed = false;

	SetVisible(false);
	return true;
}

void LapReadyGate::Finalize() {
}

void LapReadyGate::Update(double deltaTime) {

	auto racingBoats = m_scene->GetGameObjects<RacingBoat>();

	for (auto& boat : racingBoats) {
		if (boat->IsLapUpdateReady()) {
			continue;
		}

		Vector3 boatPos = boat->GetPosition();

		bool isCollided = false;

		// Z軸範囲内かチェック
		float gateHalfWidth = m_scale.x * 0.5f;

		// ボートの中心がゲートの幅内にあるかチェック
		if (boatPos.z > m_position.z - gateHalfWidth && boatPos.z < m_position.z + gateHalfWidth) {
			// X軸範囲内かチェック
			float collisionThickness = m_scale.z * 0.5f + boat->GetLength() * 0.5f;

			// ボートの中心がゲートの厚み範囲内にあるかチェック
			if (std::abs(boatPos.x - m_position.x) < collisionThickness) {
				isCollided = true;
			}
		}

		// 進行方向のチェック
		if (isCollided) {
			Vector3 boatVelocity = boat->GetVelocity();
			boatVelocity.y = 0.0f; // 水平成分のみ

			// ゲートの正面方向ベクトル
			Vector3 gateForward = GetForward();

			// 内積を計算して、進行方向がゲートの正面方向と異なれば通過とみなす
			if (boatVelocity.Dot(gateForward) < 0.0f) {
				boat->SetLapUpdateReady(true);
			}
		}

	}
}

void LapReadyGate::Draw() {
}
