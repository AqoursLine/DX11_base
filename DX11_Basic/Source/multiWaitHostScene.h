#pragma once

#include "scene.h"

#include "webClient.h"

class MultiWaitUser;

class MultiWaitHostScene : public Scene {
public:
	MultiWaitHostScene() = default;
	~MultiWaitHostScene() = default;

protected:
	bool Initialize() override;
	void Activate() override {}
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override {}
	void CleanUp() override {}


private:
	void ReceiveMessages(const json& message);

	WebClient* m_webClient = nullptr;

	std::vector<std::string> m_playerNames;

	std::string m_roomId = "";
	bool m_roomCreated = false;

	int m_connectedPlayerCount = 0;

	bool m_isStartSignalSent = false;

	float m_changeSceneTimer = 0.0f;

	std::vector<MultiWaitUser*> m_waitUsers;
};
