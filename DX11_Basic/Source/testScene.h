#pragma once

#include "scene.h"

class TestScene : public Scene {
public:
	TestScene() = default;
	~TestScene() = default;

protected:
	bool Initialize() override;
	void Update(double deltaTime) override;
private:

};