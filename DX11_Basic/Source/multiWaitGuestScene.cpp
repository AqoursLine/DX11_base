#include "main.h"
#include "multiWaitGuestScene.h"
#include "system.h"
#include "manager.h"

#include "multiWaitUser.h"
#include "startRaceText.h"

#include "multiGameGuestScene.h"
#include "testTransition.h"

#include "input.h"

bool MultiWaitGuestScene::Initialize() {
	// ウェブクライアント取得
	m_webClient = SYSTEM.GetWebClient();

	bool isConnected = m_webClient->IsConnected();

	if (isConnected) {
		m_webClient->SetOnMessage([this](const json& message) {
			ReceiveMessages(message);
		});

		json message;
		message["request"] = "joinRoom";
		message["playerName"] = "GuestPlayer"; // プレイヤー名を適宜設定
		m_webClient->SendMessageClient(message);
	}

	// 接続プレイヤー数初期化
	m_connectedPlayerCount = 0;

	// ユーザー表示オブジェクトを追加
	Vector3 userPosition = { 700.0f, 400.0f, 0.0f };
	Vector3 userScale = { 400.0f, 150.0f, 1.0f };
	Vector3 userRotation = { 0.0f, 0.0f, 0.0f };

	for (int i = 0; i < 6; i++) {
		Vector3 deltaPos = { (i % 2) * 520.0f, (i / 2) * 200.0f, 0.0f };

		auto user = AddGameObject<MultiWaitUser>(TYPE_BEFORE_PROCESS_UI)->SetIconVisible(i == 0)->SetReady(i == 0);
		user->SetPosition(userPosition + deltaPos)->SetScale(userScale)->SetRotation(userRotation);
		m_waitUsers.push_back(user);
	}

	// スタートレーステキスト追加
	AddGameObject<StartRaceText>(TYPE_BEFORE_PROCESS_UI);
	return true;
}

void MultiWaitGuestScene::Finalize() {
	// 退出メッセージを送信
	if (m_roomJoined && !m_receivedStartSignal && !m_roomClosed) {
		json message;
		message["type"] = "guestLeave";
		message["guestNumber"] = m_guestNumber;
		m_webClient->SendMessageClient(message);
	}
}

void MultiWaitGuestScene::Update(double deltaTime) {
	// 準備完了
	if (Input::GetKeyTrigger(KK_ENTER)) {
		json message;
		message["type"] = "guestReady";
		message["guestNumber"] = m_guestNumber;
		m_webClient->SendMessageClient(message);

		m_waitUsers[m_guestNumber]->SetReady(true);
	}

	// 開始通知の表示更新
	auto startRaceText = GetGameObject<StartRaceText>();
	startRaceText->SetReady(m_receivedStartSignal);

	// 開始通知を受け取った場合、一定時間後にゲームシーンへ移行
	if (m_receivedStartSignal) {
		m_changeSceneTimer += static_cast<float>(deltaTime);

		// タイマーが1秒を超えたらシーン切り替え
		if (m_changeSceneTimer >= 1.0f) {
			auto multiGameGuestScene = new MultiGameGuestScene();

			multiGameGuestScene->SetPlayerCount(m_connectedPlayerCount);
			multiGameGuestScene->SetUserId(m_guestNumber);
			SYSTEM.GetManager()->SetScene(multiGameGuestScene, new TestTransition());
		}
	}
}

void MultiWaitGuestScene::ReceiveMessages(const json& message) {
	// メッセージの種類に応じた処理
	if (!message.contains("type")) {
		// "type"フィールドが存在しない場合は無視
		return;
	}
	std::string responseType = message["type"];

	// 部屋に参加した場合の処理
	if (responseType == "roomJoined") {
		m_roomId = message["roomId"];
		m_roomJoined = true;
		m_guestNumber = message["guestNumber"];
		// 準備完了しているビットフラグを取得
		int isReadyMembers = message["readyMember"];

		for (int i = 0; i < m_guestNumber + 1; i++) {
			m_waitUsers[i]->SetIconVisible(true);
			if (isReadyMembers & (1 << i)) {
				m_waitUsers[i]->SetReady(true);

			}
			m_connectedPlayerCount++;
		}
	}

	// 新しいゲストが参加した場合の処理
	if (responseType == "newGuestJoined") {
		std::string playerName = message["playerName"];
		m_playerNames.push_back(playerName);

		m_waitUsers[message["guestNumber"]]->SetIconVisible(true);

		m_connectedPlayerCount++;
	}

	// 他の参加者の準備状態が更新された場合の処理
	if (responseType == "guestReady") {
		int guestNumber = message["guestNumber"];
		m_waitUsers[guestNumber]->SetReady(true);
	}

	// ホストからの開始通知を受け取った場合
	if (responseType == "hostStart") {
		// ゲーム開始処理を実行
		m_receivedStartSignal = true;
	}

	// プレイヤーが退出した場合の処理
	if (responseType == "guestLeft") {
		int guestNumber = message["guestNumber"];
		m_waitUsers[guestNumber]->SetIconVisible(false);
		m_waitUsers[guestNumber]->SetReady(false);
		m_connectedPlayerCount--;
	}

	// 部屋が閉鎖された場合の処理
	if (responseType == "roomClosed") {
		m_roomClosed = true;
	}
}
