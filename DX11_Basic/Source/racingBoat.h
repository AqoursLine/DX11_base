#pragma once

#include "boat.h"

class RacingBoat : public Boat {
public:
	RacingBoat() = default;
	~RacingBoat() = default;


	void SetThrottle(float throttle) override;

	void SetStarting(bool isStarting) { m_isStarted = isStarting; }

protected:
	bool Initialize() override;

	Vector2 GetSceneBoundsMin() override;
	Vector2 GetSceneBoundsMax() override;

private:
	class RaceManager* m_raceManager = nullptr;

	bool m_isStarted = false;
};
