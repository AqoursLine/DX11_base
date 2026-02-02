#pragma once

#include "player.h"

class MultiPlayer : public Player {
public:
	MultiPlayer() = default;
	~MultiPlayer() = default;

	// プレイヤーID設定
	void SetPlayerId(int playerId) { m_playerId = playerId; }
protected:
	bool Initialize() override;
	void Update(double deltaTime) override;

private:
	int m_playerId = -1;
};

