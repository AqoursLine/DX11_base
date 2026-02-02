#pragma once

#include "scene.h"

#include "webClient.h"

class MultiOtherPlayer;

class MultiGameGuestScene : public Scene {
public:
	MultiGameGuestScene() = default;
	~MultiGameGuestScene() = default;

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

	int m_playerCount = 0;
	int m_userId = -1;

	std::array<MultiOtherPlayer*, 6> m_otherPlayers;
	int m_readyUserCount = 0;

	bool m_isActivated = false;

};
