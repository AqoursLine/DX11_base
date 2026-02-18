#pragma once

#include "scene.h"

class ResultScene : public Scene {
public:
	ResultScene() = default;
	~ResultScene() = default;

protected:
	bool Initialize() override;
	void Activate() override {}
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;
	void CleanUp() override {}

private:

};
