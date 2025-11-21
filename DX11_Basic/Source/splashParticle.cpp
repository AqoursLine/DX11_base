#include "splashParticle.h"
#include "texture.h"
#include "input.h"
#include "imguiSystem.h"

bool SplashParticle::Initialize() {
	m_texture = new Texture();
	if (!m_texture) {
		return false;
	}

	if (!m_texture->Load(L"Asset\\Texture\\circle.png")) {
		return false;
	}

	SetTexture(m_texture->GetSRV());

	EmitterSettings settings;
	settings.startColor = { 0.6f, 0.8f, 1.0f, 0.9f };
	settings.endColor = { 0.4f, 0.6f, 0.8f, 0.0f };
	settings.startSize = 0.2f;
	settings.endSize = 0.0f;
	settings.lifeTime = 1.2f;
	settings.position = { 0.5f, 0.0f, 0.5f };
	settings.velocity = { 0.0f, 3.0f, 0.0f };
	settings.velocityVariation = { 2.5f, 1.0f, 2.5f };
	settings.gravity = -9.8f;
	settings.maxParticles = 50000;
	settings.oneShot = true;
	settings.oneShotCount = 20;

	SetEmitterSettings(settings);

	ParticleSystem::Initialize();
	return true;
}

void SplashParticle::Finalize() {
	if (m_texture) {
		delete m_texture;
		m_texture = nullptr;
	}

	ParticleSystem::Finalize();
}

void SplashParticle::Update(double deltaTime) {
#ifdef _DEBUG
	if (Input::GetKeyTrigger(KK_D1)) {
		EmitOneShot(m_position);
		EmitOneShot({ -5.0f, 0.0f, 0.0f });
	}
#endif // _DEBUG


	ParticleSystem::Update(deltaTime);
}

void SplashParticle::Draw() {
//#ifdef _DEBUG
//	// ImGuiでエミッター設定を調整
//	ImGui::Begin("Splash Particle Emitter Settings");
//	EmitterSettings& settings = GetEmitterSettings();
//	ImGui::ColorEdit4("Start Color", (float*)&settings.startColor);
//	ImGui::ColorEdit4("End Color", (float*)&settings.endColor);
//	ImGui::SliderFloat("Start Size", &settings.startSize, 0.1f, 5.0f, "%.1f");
//	ImGui::SliderFloat("End Size", &settings.endSize, 0.0f, 5.0f, "%.1f");
//	ImGui::SliderFloat("Life Time", &settings.lifeTime, 0.1f, 10.0f, "%.1f");
//	ImGui::SliderFloat3("Velocity", (float*)&settings.velocity, -10.0f, 10.0f, "%.1f");
//	ImGui::SliderFloat3("Velocity Variation", (float*)&settings.velocityVariation, 0.0f, 10.0f, "%.1f");
//	ImGui::SliderFloat("Gravity", &settings.gravity, -20.0f, 0.0f, "%.1f");
//	ImGui::End();
//
//#endif // _DEBUG


	ParticleSystem::Draw();
}
