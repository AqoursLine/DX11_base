#include "racingBoat.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "raceManager.h"

void RacingBoat::SetThrottle(float throttle) {
	float adjustedThrottle = throttle;

	if (m_isStarted) {
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

bool RacingBoat::Initialize() {
	m_rotation = { 0.0f, XM_PIDIV2, 0.0f };
	m_position = { -160.0f, 0.0f, -50.0f };

	//ボートの初期方向をセット
	SetStartYaw(m_rotation.y);

	//レースマネージャ取得
	m_raceManager = SYSTEM.GetManager()->GetScene()->GetGameObject<RaceManager>();

	return Boat::Initialize();
}

Vector2 RacingBoat::GetSceneBoundsMin() {
	return m_raceManager->GetBoundsMin();
}

Vector2 RacingBoat::GetSceneBoundsMax() {
	return m_raceManager->GetBoundsMax();
}
