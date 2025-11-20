#include "raceManager.h"

#include "system.h"
#include "manager.h"
#include "scene.h"
#include "racingBoat.h"

#include "webClient.h"
#include "myRandom.h"

#include "resultScene.h"

//静的メンバ変数の初期化
std::vector<BoatResultData> RaceManager::m_result;

void RaceManager::SetResultData(const BoatResultData& data) {
	m_result.push_back(data);

	//データにゴールタイムを設定
	m_result.back().finishTime = m_raceTime;

	//全ボートが完走したらレース終了
	if (m_result.size() >= m_racingBoats.size()) {
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

	m_result.clear();

	//ボートの順番をシャッフル
	std::shuffle(m_racingBoats.begin(), m_racingBoats.end(), MyRandom::GetEngine());

	//ボートに番号を割り当て
	int laneIndex = 0;
	for (auto& boat : m_racingBoats) {
		//レーンインデックス設定
		boat->SetLaneIndex(laneIndex);

		//初期位置設定
		float laneSpacing = 10.0f; //レーン間隔
		float startx = -160.0f; //スタート位置X座標
		float startz = -(5.0f + laneIndex * laneSpacing);

		boat->SetPosition({ startx, 0.0f, startz });

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

			//結果シーンへ移行
			SYSTEM.GetManager()->SetScene(new ResultScene());

			// 結果を送信
			auto webClient = SYSTEM.GetWebClient();

			if (webClient->IsConnected()) {
				json message;
				message["type"] = "raceResult";
				message["results"] = json::array();
				for (const auto& resultData : m_result) {
					json resultJson;
					resultJson["laneIndex"] = resultData.laneIndex;
					resultJson["finishTime"] = resultData.finishTime;
					resultJson["boatColor"] = { resultData.boatColor.x, resultData.boatColor.y, resultData.boatColor.z, resultData.boatColor.w };
					resultJson["playerName"] = resultData.playerName;
					message["results"].push_back(resultJson);
				}
				webClient->SendMessageClient(message);
			}

			//リセット
			m_raceFinished = false;
			m_raceTime = 0.0f;
			m_raceStarted = false;
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
