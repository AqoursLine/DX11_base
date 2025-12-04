#pragma once
#include "scene.h"

class GameScene : public Scene {
public:
	GameScene() = default;
	~GameScene() = default;

protected:
	bool Initialize() override;
	void Activate() override {}
	void Finalize() override {}
	void Update(double deltaTime) override {};
	void Draw() override {}
	void CleanUp() override {}

private:

};

