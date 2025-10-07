#include "raceManager.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "racingBoat.h"

bool RaceManager::Initialize() {
	//初期化処理

	//シーンからレース参加ボートを取得
	auto scene = SYSTEM.GetManager()->GetScene();
	m_racingBoats = scene->GetGameObjects<RacingBoat>();

	//描画オフ
	SetVisible(false);

	return true;
}

void RaceManager::Finalize() {
}

void RaceManager::Update(double deltaTime) {
	//スタート前のカウントダウン
	if (m_startDelay > 0.0f) {
		m_startDelay -= static_cast<float>(deltaTime);

		if (m_startDelay <= 0.0f) {
			for (auto& boat : m_racingBoats) {
				boat->SetStarting(true);
			}
		}
	} else {
		//レースタイム更新
		m_raceTime += static_cast<float>(deltaTime);
	}
}

void RaceManager::Draw() const {
}

void RaceManager::CleanUp() {
}
