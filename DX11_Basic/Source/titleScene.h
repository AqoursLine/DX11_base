#pragma once
#include "scene.h"

class TitleScene : public Scene {
public:
	TitleScene() = default;
	~TitleScene() = default;

	bool Initialize() override;
	void Update(double deltaTime) override;
private:

};
