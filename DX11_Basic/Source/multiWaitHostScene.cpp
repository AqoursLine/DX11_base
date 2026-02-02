#include "main.h"
#include "multiWaitHostScene.h"
#include "system.h"
#include "manager.h"
#include "multiGameHostScene.h"
#include "testTransition.h"

#include "player.h"

#include "multiWaitUser.h"
#include "readyButton.h"

#include "input.h"


bool MultiWaitHostScene::Initialize() {
	// ウェブクライアント取得
	m_webClient = SYSTEM.GetWebClient();

	bool isConnected = m_webClient->IsConnected();

	if (isConnected) {
		m_webClient->SetOnMessage([this](const json& message) {
			ReceiveMessages(message);
		});

		json message;
		message["request"] = "createRoom";
		m_webClient->SendMessageClient(message);
	}

	// 接続プレイヤー数初期化(ホスト含める)
	m_connectedPlayerCount = 1;

	// ユーザー表示オブジェクトを追加
	Vector3 userPosition = { 700.0f, 400.0f, 0.0f};
	Vector3 userScale = { 400.0f, 150.0f, 1.0f };
	Vector3 userRotation = { 0.0f, 0.0f, 0.0f };

	for (int i = 0; i < 6; i++) {
		Vector3 deltaPos = { (i % 2) * 520.0f, (i / 2) * 200.0f, 0.0f };

		auto user = AddGameObject<MultiWaitUser>(TYPE_BEFORE_PROCESS_UI)->SetIconVisible(i == 0)->SetReady(i == 0);
		user->SetPosition(userPosition + deltaPos)->SetScale(userScale)->SetRotation(userRotation);
		m_waitUsers.push_back(user);
	}

	// スタートボタン追加
	auto readyButton = AddGameObject<ReadyButton>(TYPE_BEFORE_PROCESS_UI);
	readyButton->SetPosition({ 960.0f, 970.0f, 0.0f });

	return true;
}

void MultiWaitHostScene::Finalize() {
}

void MultiWaitHostScene::Update(double deltaTime) {
	// 全てが準備完了したら開始ボタンを有効化
	for (auto& waitUser : m_waitUsers) {
		if (!waitUser->IsReady() && waitUser->IsIconVisible()) {
			return;
		}
	}

	auto readyButton = GetGameObject<ReadyButton>();
	readyButton->SetReady(true);

	// ゲーム開始
	if (Input::GetKeyTrigger(KK_ENTER)) {
		json message;
		message["type"] = "hostStart";
		m_webClient->SendMessageClient(message);

		// ゲームシーンへ移行
		auto multiGameHostScene = new MultiGameHostScene();
		multiGameHostScene->SetPlayerCount(m_connectedPlayerCount);
		multiGameHostScene->SetUserId(0);
		SYSTEM.GetManager()->SetScene(multiGameHostScene, new TestTransition());
	}

}

void MultiWaitHostScene::ReceiveMessages(const json& message) {
	// メッセージの種類に応じた処理
	if (!message.contains("type")) {
		// "type"フィールドが存在しない場合は無視
		return;
	}

	std::string responseType = message["type"];

	// 部屋が作成された場合の処理
	if (responseType == "roomCreated") {
		m_roomId = message["roomId"];
		m_roomCreated = true;
	} 

	// プレイヤーが参加した場合の処理
	if (responseType == "guestJoined") {
		std::string playerName = message["playerName"];
		// プレイヤーリストに追加するなどの処理を行う
		m_playerNames.push_back(playerName);
		m_waitUsers[message["guestNumber"]]->SetIconVisible(true);
		m_connectedPlayerCount++;
	}

	// プレイヤーが準備完了した場合の処理
	if (responseType == "guestReady") {
		m_waitUsers[message["guestNumber"]]->SetReady(true);
	}

}
