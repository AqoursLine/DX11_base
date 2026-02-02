#include "multiPlayer.h"

#include "system.h"
#include "webClient.h"

bool MultiPlayer::Initialize() {
	Player::Initialize();
    return true;
}

void MultiPlayer::Update(double deltaTime) {
	Player::Update(deltaTime);

	json message;
	message["type"] = "playerUpdate";
	message["userId"] = m_playerId;
	message["position"] = {
		{ "x", m_position.x },
		{ "y", m_position.y },
		{ "z", m_position.z }
	};
	message["quaternion"] = {
		{ "x", m_quaternion.x },
		{ "y", m_quaternion.y },
		{ "z", m_quaternion.z },
		{ "w", m_quaternion.w }
	};
	Vector3 velocity = GetVelocity();
	message["velocity"] = {
		{ "x", velocity.x },
		{ "y", velocity.y },
		{ "z", velocity.z }
	};
	SYSTEM.GetWebClient()->SendMessageClient(message);
}
