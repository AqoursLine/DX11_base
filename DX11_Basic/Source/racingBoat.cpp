#include "racingBoat.h"

void RacingBoat::SetThrottle(float throttle) {
	float adjustedThrottle = throttle;

	if (m_isStarted) {
		//スタート開始後
		if (throttle < 0) {
			//リバースギア
			SetReverse(true);
		} else {
			SetReverse(false);
		}

		throttle = std::abs(throttle);
	} else {
		throttle = 0.0f;
	}

	Boat::SetThrottle(throttle);
}
