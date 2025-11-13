#include "raceManager.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "racingBoat.h"

#include "resultScene.h"

//静的メンバ変数の初期化
std::vector<float> RaceManager::m_laneTime;

void RaceManager::SetLaneTime(int laneIndex) {
	if (laneIndex >= 0 && laneIndex < static_cast<int>(m_laneTime.size())) {
		m_laneTime[laneIndex] = m_raceTime;
	} else if (laneIndex == static_cast<int>(m_laneTime.size())) {
		//新しいレーンのタイムを追加
		m_laneTime.push_back(m_raceTime);
	}

	//デバッグ
#ifdef _DEBUG
	std::cout << "\n\n\n\n";
	std::cout << "Lane " << laneIndex << " Time: " << m_raceTime << " seconds" << std::endl;

#endif // _DEBUG


	if (m_laneTime.size() >= m_racingBoats.size()) {
		m_raceFinished = true;
		m_raceTime = 0.0f;
	}
}

bool RaceManager::Initialize() {
	//初期化処理

	//シーンからレース参加ボートを取得
	m_racingBoats = m_scene->GetGameObjects<RacingBoat>();

	//描画オフ
	SetVisible(false);

	m_laneTime.clear();

	//ボートに番号を割り当て
	int laneIndex = 2;
	for (auto& boat : m_racingBoats) {
		boat->SetLaneIndex(laneIndex);
		laneIndex++;
	}

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
