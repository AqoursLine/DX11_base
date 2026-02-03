#pragma once

#include "scene.h"

class TestScene : public Scene {
public:
	TestScene() = default;
	~TestScene() = default;

protected:
	bool Initialize() override;
	void Activate() override {}
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;
	void CleanUp() override {}
private:
	float m_testTimer = 0.0f;
	float m_deltaTime = 0.0;

	int m_triggeredG = 0;
};