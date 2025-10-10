#pragma once

#include "scene.h"

class ResultScene : public Scene {
public:
	ResultScene() = default;
	~ResultScene() = default;

	bool Initialize() override;
	void Update(double deltaTime) override;

private:

};
