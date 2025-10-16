#pragma once

#include "light.h"

class GameDirectionalLight : public Light {
public:
	GameDirectionalLight() = default;
	~GameDirectionalLight() = default;

protected:
	bool Initialize() override;
	void Update(double deltaTime) override;

private:

};

