#pragma once

#include "camera.h"

class FpsCamera : public Camera {
public:
	FpsCamera() = default;
	~FpsCamera() = default;

	void Update(double deltaTime) override;

protected:
	void RotattionCamera(double deltaTime);

};
