#include "main.h"
#include "multiWaitGuestScene.h"
#include "system.h"

#include "multiWaitUser.h"

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

	// 接続プレイヤー数初期化(自身とホスト含める)
	m_connectedPlayerCount = 1;

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


	return true;
}

void MultiWaitGuestScene::Finalize() {
	// メッセージ受信のコールバックをリセット
	m_webClient->SetOnMessage([](const json& message) {
		std::cout << "Received message: " << message.dump() << std::endl;
		});

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
}

void MultiWaitGuestScene::ReceiveMessages(const json& message) {
	// メッセージの種類に応じた処理
	if (!message.contains("type")) {
		// "type"フィールドが存在しない場合は無視
		return;
	}
	std::string responseType = message["type"];
	if (responseType == "roomJoined") {
		// 部屋に参加した場合の処理
		m_roomId = message["roomId"];
		m_roomJoined = true;
		m_guestNumber = message["guestNumber"];

		m_waitUsers[m_guestNumber]->SetIconVisible(true);

	} else if (responseType == "newPlayerJoined") {
		// 新しいプレイヤーが参加した場合の処理
		std::string playerName = message["playerName"];
		// プレイヤーリストに追加するなどの処理を行う
		m_playerNames.push_back(playerName);
	}
}
