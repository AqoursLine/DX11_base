#pragma once

#include "scene.h"

#include "webClient.h"

class MultiOtherPlayer;

class MultiGameHostScene : public Scene {
public:
	MultiGameHostScene() = default;
	~MultiGameHostScene() = default;

	void SetPlayerCount(int count) { m_playerCount = count; }
	void SetUserId(int id) { m_userId = id; }

protected:
	virtual bool Initialize() override;
	virtual void Activate() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() override;
	virtual void CleanUp() override;

private:
	void ReceiveMessages(const json& message);

	WebClient* m_webClient = nullptr;

	// プレイヤー数
	int m_playerCount = 0;
	// 自分のユーザーID
	int m_userId = -1;

	std::array<MultiOtherPlayer*, 6> m_otherPlayers;
	int m_readyUserCount = 0;

	// シーンがアクティブ化されたかどうか
	bool m_isActivated = false;

	// 全ゲストの準備完了フラグ
	bool m_allGuestsReady = false;

	// レース開始フラグ
	bool m_raceStarted = false;

	// タイマー同期間隔(秒)
	float m_timerSyncInterval = 0.5f;
	// タイマー同期経過時間(秒)
	float m_timerSyncElapsed = 0.0f;


};
