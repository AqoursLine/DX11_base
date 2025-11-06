#pragma once
#include "scene.h"

class TitleScene : public Scene {
public:
	TitleScene() = default;
	~TitleScene() = default;

protected:
	bool Initialize() override;
	void Finalize() override {};
	void Update(double deltaTime) override;
	void Draw() override {};
	void CleanUp() override {};
private:

};
