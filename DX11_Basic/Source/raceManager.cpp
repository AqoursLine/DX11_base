#include "raceManager.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "racingBoat.h"

#include "resultScene.h"

//静的メンバ変数の初期化
std::unordered_map<int, float> RaceManager::m_laneTime;

void RaceManager::SetLaneTime(int laneIndex) {
	m_laneTime[laneIndex] = m_raceTime;

	if (m_laneTime.size() >= m_racingBoats.size()) {
		m_raceFinished = true;
		m_raceTime = 0.0f;
	}
}

bool RaceManager::Initialize() {
	//初期化処理

	//シーンからレース参加ボートを取得
	auto scene = SYSTEM.GetManager()->GetScene();
	m_racingBoats = scene->GetGameObjects<RacingBoat>();

	//描画オフ
	SetVisible(false);

	m_laneTime.clear();

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
	} else if (m_raceFinished) {
		m_raceTime += static_cast<float>(deltaTime);

		if (m_raceTime >= 3.0f) {
			SYSTEM.GetManager()->SetScene(new ResultScene());
		}
	} else if (m_raceStarted) {
		//レースタイム更新
		m_raceTime += static_cast<float>(deltaTime);
	}
}

void RaceManager::Draw() const {
}

void RaceManager::CleanUp() {
}
