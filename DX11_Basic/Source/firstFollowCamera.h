#pragma once

#include "camera.h"

class FirstFollowCamera : public Camera {
public:
	FirstFollowCamera() = default;
	~FirstFollowCamera() = default;
protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() override;
	virtual void CleanUp() override;
private:
	class RaceManager* m_raceManager = nullptr;

	void UpdatePosition(class RacingBoat* targetBoat, double deltaTime);
};
