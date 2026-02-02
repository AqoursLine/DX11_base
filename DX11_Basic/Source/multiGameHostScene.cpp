#include "main.h"
#include "multiGameHostScene.h"

#include "system.h"

#include "multiPlayer.h"
#include "multiOtherPlayer.h"

#include "wall.h"
#include "fieldObject.h"
#include "water.h"
#include "buoy.h"
#include "raceManager.h"
#include "countDownText.h"
#include "miniMap.h"
#include "speedMeter.h"
#include "startGate.h"
#include "goalGate.h"
#include "lapReadyGate.h"
#include "raceTimer.h"
#include "lapDisplay.h"
#include "rankDisplay.h"
#include "gameDirectionalLight.h"
#include "skyDome.h"
#include "splashParticle.h"

#include "camera.h"

bool MultiGameHostScene::Initialize() {
	// ウェブクライアント取得
	m_webClient = SYSTEM.GetWebClient();

	// メッセージ受信のコールバック設定
	m_webClient->SetOnMessage([this](const json& message) {
		ReceiveMessages(message);
		});

	//とりあえずの壁
	//右壁
	AddGameObject<Wall>(TYPE_OPAQUE)->SetPosition({ 220.0f, 5.0f, 0.0f })->SetScale({ 10.0f, 20.0f, 130.0f })->SetRotation({ 0.0f, XM_PI, 0.0f });
	//左壁
	AddGameObject<Wall>(TYPE_OPAQUE)->SetPosition({ -220.0f, 5.0f, 0.0f })->SetScale({ 10.0f, 20.0f, 130.0f })->SetRotation({ 0.0f, XM_PI, 0.0f });
	//前壁
	AddGameObject<Wall>(TYPE_OPAQUE)->SetPosition({ 0.0f, 5.0f, 65.0f })->SetScale({ 400.0f, 20.0f, 10.0f })->SetRotation({ 0.0f, XM_PI, 0.0f });
	//後壁
	AddGameObject<Wall>(TYPE_OPAQUE)->SetPosition({ 0.0f, 5.0f, -65.0f })->SetScale({ 400.0f, 20.0f, 10.0f })->SetRotation({ 0.0f, XM_PI, 0.0f });

	//フィールドオブジェクト
	AddGameObject<FieldObject>(TYPE_OPAQUE);

	//水
	AddGameObject<Water>(TYPE_TRANSPARENT);

	//プレイヤー
	for (int i = 0; i < m_playerCount; i++) {
		if (i == m_userId) {
			AddGameObject<MultiPlayer>(TYPE_OPAQUE)->SetPlayerId(m_userId);
		} else {
			m_otherPlayers[i] = AddGameObject<MultiOtherPlayer>(TYPE_OPAQUE);
		}
	}

	//浮き
	AddGameObject<Buoy>(TYPE_OPAQUE)->SetPosition({ 150.0f, 0.0f, 0.0f });
	AddGameObject<Buoy>(TYPE_OPAQUE)->SetPosition({ -150.0f, 0.0f, 0.0f });

	//レースマネージャー
	AddGameObject<RaceManager>(TYPE_CUTOUT);

	//カウントダウン
	AddGameObject<RaceCountDownText>(TYPE_BEFORE_PROCESS_UI);

	//ミニマップ
	AddGameObject<MiniMap>(TYPE_BEFORE_PROCESS_UI);

	//スピードメーター
	AddGameObject<SpeedMeter>(TYPE_BEFORE_PROCESS_UI);

	//スタートゲート
	AddGameObject<StartGate>(TYPE_CUTOUT);

	//ゴールゲート
	AddGameObject<GoalGate>(TYPE_CUTOUT);

	//ラップゲート
	AddGameObject<LapReadyGate>(TYPE_CUTOUT);

	//レースタイマー
	AddGameObject<RaceTimer>(TYPE_BEFORE_PROCESS_UI);

	//ラップ表示
	AddGameObject<LapDisplay>(TYPE_BEFORE_PROCESS_UI);

	//順位表示
	AddGameObject<RankDisplay>(TYPE_BEFORE_PROCESS_UI);

	//カメラの初期化
	AddGameObject<Camera>(TYPE_CAMERA);

	//スカイドーム
	AddGameObject<SkyDome>(TYPE_OPAQUE);

	//平行光源
	AddGameObject<GameDirectionalLight>(TYPE_LIGHT)->SetDirection({ -1.0f, -1.0f, -1.0f, 0.0f })->SetEnabled(true);

	//スプラッシュエフェクト
	AddGameObject<SplashParticle>(TYPE_TRANSPARENT);

	return true;
}

void MultiGameHostScene::Activate() {
	m_isActivated = true;
}

void MultiGameHostScene::Finalize() {
	// メッセージ受信のコールバックをリセット
	m_webClient->SetOnMessage([](const json& message) {
		std::cout << "Received message: " << message.dump() << std::endl;
		});
}

void MultiGameHostScene::Update(double deltaTime) {
	// 全員が準備完了したらレース開始
	if (m_allGuestsReady && m_isActivated && !m_raceStarted) {
		// 全員にレース開始メッセージを送信
		json startMessage;
		startMessage["type"] = "startRace";
		m_webClient->SendMessageClient(startMessage);

		m_raceStarted = true;
	}
}

void MultiGameHostScene::Draw() {
}

void MultiGameHostScene::CleanUp() {
}

void MultiGameHostScene::ReceiveMessages(const json& message) {
	if (!message.contains("type")) {
		return;
	}

	std::string responseType = message["type"];

	if (responseType == "guestCompletedSceneChange") {
		// ゲームシーンへの移行完了通知
		int userId = message["userId"];
		if (userId < 0 || userId >= m_otherPlayers.size()) {
			return;
		}

		m_readyUserCount++;
		// 全員が準備完了したらレース開始(ホスト以外)
		if (m_readyUserCount == m_playerCount - 1) {
			m_allGuestsReady = true;
		}
	}

	// プレイヤー情報更新
	if (responseType == "playerUpdate" && m_isActivated) {
		int userId = message["userId"];
		if (userId < 0 || userId >= m_otherPlayers.size()) {
			return;
		}
		m_otherPlayers[userId]->SetDataFromNetwork(message);
	}

	// スタート
	if (responseType == "startRace") {
		auto raceManager = GetGameObject<RaceManager>();
		raceManager->SetSceneStarted(true);
	}
}
