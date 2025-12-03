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
	settings.startColor = { 0.0f, 0.5f, 1.0f, 1.0f };
	settings.endColor = { 0.4f, 0.6f, 0.8f, 1.0f };
	settings.startSize = 0.2f;
	settings.endSize = 0.0f;
	settings.lifeTime = 1.2f;
	settings.position = { 0.5f, 0.0f, 0.5f };
	settings.velocity = { 0.0f, 2.0f, 0.0f };
	settings.velocityVariation = { 2.5f, 1.0f, 2.5f };
	settings.gravity = -9.8f;
	settings.maxParticles = 5000;
	settings.oneShot = true;
	settings.oneShotCount = 10;

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
