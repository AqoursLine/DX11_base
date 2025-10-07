#pragma once

#include "boat.h"

class RacingBoat : public Boat {
public:
	RacingBoat() = default;
	~RacingBoat() = default;

	void SetThrottle(float throttle) override;

	void SetStarting(bool isStarting) { m_isStarted = isStarting; }

private:

	bool m_isStarted = false;
};
