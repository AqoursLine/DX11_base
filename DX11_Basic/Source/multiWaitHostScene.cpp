#include "main.h"
#include "multiWaitHostScene.h"
#include "system.h"

#include "player.h"

#include "multiWaitUser.h"


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

	return true;
}

void MultiWaitHostScene::Finalize() {
}

void MultiWaitHostScene::Update(double deltaTime) {
	// 全てが準備完了したら開始ボタンを有効化
	for (auto& waitUser : m_waitUsers) {
		if (!waitUser->IsReady()) {
			return;
		}
	}

}

void MultiWaitHostScene::ReceiveMessages(const json& message) {
	// メッセージの種類に応じた処理
	if (!message.contains("type")) {
		// "type"フィールドが存在しない場合は無視
		return;
	}

	std::string responseType = message["type"];
	if (responseType == "roomCreated") {
		// 部屋が作成された場合の処理
		m_roomId = message["roomId"];
		m_roomCreated = true;
	} else if (responseType == "guestJoined") {
		// プレイヤーが参加した場合の処理
		std::string playerName = message["playerName"];
		// プレイヤーリストに追加するなどの処理を行う
		m_playerNames.push_back(playerName);
		m_waitUsers[message["guestNumber"]]->SetIconVisible(true);
		m_connectedPlayerCount++;
	} else if (responseType == "guestReady") {
		m_waitUsers[message["guestNumber"]]->SetReady(true);
	}

}
