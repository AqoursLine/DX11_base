#pragma once

#include "scene.h"

#include "webClient.h"

class MultiWaitUser;

class MultiWaitGuestScene : public Scene {
public:
	MultiWaitGuestScene() = default;
	~MultiWaitGuestScene() = default;

protected:
	bool Initialize() override;
	void Activate() override {}
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override {}
	void CleanUp() override {}


private:
	void ReceiveMessages(const json& message);

	class WebClient* m_webClient = nullptr;

	std::vector<std::string> m_playerNames;

	std::string m_roomId = "";
	bool m_roomJoined = false;
	int m_guestNumber = 0;
	bool m_receivedStartSignal = false;
	bool m_roomClosed = false;

	int m_connectedPlayerCount = 0;

	float m_changeSceneTimer = 0.0f;

	std::vector<MultiWaitUser*> m_waitUsers;
};
