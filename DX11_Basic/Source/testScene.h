#pragma once

#include "scene.h"

class TestScene : public Scene {
public:
	TestScene() = default;
	~TestScene() = default;

protected:
	bool Initialize() override;
	void Finalize() override {};
	void Update(double deltaTime) override;
	void Draw() override {};
	void CleanUp() override {};
private:

};