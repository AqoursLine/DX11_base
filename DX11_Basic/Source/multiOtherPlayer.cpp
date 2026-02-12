#include "multiOtherPlayer.h"

void MultiOtherPlayer::SetDataFromNetwork(const json& data) {
	m_position.x = data["position"]["x"];
	m_position.y = data["position"]["y"];
	m_position.z = data["position"]["z"];
	m_quaternion = Vector4(
		data["quaternion"]["x"],
		data["quaternion"]["y"],
		data["quaternion"]["z"],
		data["quaternion"]["w"]
	);
	Vector3 velocity(
		data["velocity"]["x"],
		data["velocity"]["y"],
		data["velocity"]["z"]
	);
	SetVelocity(velocity);
}

bool MultiOtherPlayer::Initialize() {
	RacingBoat::Initialize();
	return true;
}

void MultiOtherPlayer::Update(double deltaTime) {
	RacingBoat::UpdateProgressSection();
	RacingBoat::CalculateLapProgress();
	Boat::UpdateWaterInteraction(static_cast<float>(deltaTime));
}
