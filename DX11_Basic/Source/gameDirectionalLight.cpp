#include "main.h"
#include "gameDirectionalLight.h"

#include "input.h"

bool GameDirectionalLight::Initialize() {
	m_type = LIGHT_TYPE::DIRECTIONAL;

	return Light::Initialize();
}

void GameDirectionalLight::Update(double deltaTime) {

}
