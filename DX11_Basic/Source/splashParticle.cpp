#include "main.h"
#include "splashParticle.h"
#include "texture.h"
#include "input.h"
#include "imguiSystem.h"

bool SplashParticle::Initialize() {
	m_texture = new Texture();
	if (!m_texture) {
		return false;
	}

	if (!m_texture->Load(L"Asset\\Texture\\particle.png")) {
		return false;
	}

	SetTexture(m_texture->GetSRV());

	EmitterSettings settings = {};
	settings.startColor = { 0.0f, 0.5f, 1.0f, 1.0f };
	settings.endColor = { 0.4f, 0.6f, 0.8f, 1.0f };
	settings.startSize = 0.1f;
	settings.endSize = 0.0f;
	settings.lifeTime = 1.2f;
	settings.position = { 0.5f, 0.0f, 0.5f };
	settings.velocity = { 0.0f, 3.0f, 0.0f };
	settings.velocityVariation = { 2.5f, 1.0f, 2.5f };
	settings.worldAcceleration = { 0.0f, -9.8f, 0.0f };
	settings.maxParticles = 24000;
	settings.oneShot = true;
	settings.oneShotCount = 300;

	SetEmitterSettings(settings);

	GPUParticleSystem::Initialize();
	return true;
}

void SplashParticle::Finalize() {
	if (m_texture) {
		delete m_texture;
		m_texture = nullptr;
	}

	GPUParticleSystem::Finalize();
}

void SplashParticle::Update(double deltaTime) {
#ifdef _DEBUG
	if (Input::GetKeyPress(KK_D8)) {
		EmitOneShot({ 0.0f, 0.0f, 0.0f });
	}
#endif // _DEBUG

	GPUParticleSystem::Update(deltaTime);
}

void SplashParticle::Draw() {
#ifdef _DEBUG
	// ImGuiでパーティクル設定を変更
	ImGui::Begin("Splash Particle Settings");
	EmitterSettings settings = GetEmitterSettings();
	bool isChanged = false;
	isChanged |= ImGui::ColorEdit4("Start Color", (float*)&settings.startColor, ImGuiColorEditFlags_AlphaBar);
	isChanged |= ImGui::ColorEdit4("End Color", (float*)&settings.endColor, ImGuiColorEditFlags_AlphaBar);
	isChanged |= ImGui::DragFloat("Start Size", &settings.startSize, 0.0f, 1.0f);
	isChanged |= ImGui::DragFloat("End Size", &settings.endSize, 0.0f, 1.0f);
	isChanged |= ImGui::DragFloat("Life Time", &settings.lifeTime, 0.0f, 5.0f);
	isChanged |= ImGui::DragFloat3("Velocity", (float*)&settings.velocity, -10.0f, 10.0f);
	isChanged |= ImGui::DragFloat3("Velocity Variation", (float*)&settings.velocityVariation, 0.0f, 10.0f);
	isChanged |= ImGui::DragFloat3("World Acceleration", (float*)&settings.worldAcceleration, -50.0f, 50.0f);
	isChanged |= ImGui::DragFloat3("Local Acceleration", (float*)&settings.localAcceleration, -50.0f, 50.0f);
	bool isChangedUpVector = false;
	isChangedUpVector |= ImGui::DragFloat3("Up Vector", (float*)&settings.upVector, -1.0f, 1.0f);
	if (isChangedUpVector) {
		settings.upVector.Normalize();
		isChanged = true;
	}
	isChanged |= ImGui::DragInt("One Shot Count", &settings.oneShotCount, 1, 1000);
	ImGui::End();

	if (isChanged) {
		SetEmitterSettings(settings);
	}
#endif // _DEBUG

	GPUParticleSystem::Draw();
}
