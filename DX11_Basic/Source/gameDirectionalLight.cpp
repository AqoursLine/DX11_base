#include "main.h"
#include "gameDirectionalLight.h"

#include "input.h"

#include "imguiSystem.h"

bool GameDirectionalLight::Initialize() {
	m_type = LIGHT_TYPE::DIRECTIONAL;

	return Light::Initialize();
}

void GameDirectionalLight::Update(double deltaTime) {
}
