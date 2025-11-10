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

		if (boatPos.z < m_position.z - m_scale.z * 0.5f && boatPos.z > m_position.z + m_scale.z * 0.5f) {
			continue;
		}

		float boatHalfLength = boat->GetLength() * 0.5f;

		if (boatPos.x + boatHalfLength > m_position.x - m_scale.x * 0.05f && boatPos.x + boatHalfLength < m_position.x + m_scale.x * 0.05f) {
			isCollided = true;
		}

		Vector3 boatVel = boat->GetVelocity();
		boatVel.y = 0.0f; //水平成分のみ

		isCollided = isCollided && (boatVel.Dot(Vector4::FromAxisAngle(Vector3::UP, m_rotation.y).RotateVector(Vector3::FORWARD)) < 0.0f);

		if (isCollided) {
			boat->SetLapUpdateReady(true);
		}
	}
}

void LapReadyGate::Draw() const {
}
