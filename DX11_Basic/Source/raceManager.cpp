#include "raceManager.h"

bool RaceManager::Initialize() {
	//初期化処理
	return true;
}

void RaceManager::Finalize() {
}

void RaceManager::Update(double deltaTime) {
	//スタート前のカウントダウン
	if (m_startDelay > 0.0f) {
		m_startDelay -= static_cast<float>(deltaTime);
	} else {
		//レースタイム更新
		m_raceTime += static_cast<float>(deltaTime);
	}
}

void RaceManager::Draw() const {
}

void RaceManager::CleanUp() {
}
