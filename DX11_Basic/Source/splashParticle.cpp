#include "splashParticle.h"
#include "texture.h"

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
	settings.startColor = { 1.0f, 0.2f, 0.2f, 1.0f };
	settings.endColor = { 1.0f, 1.0f, 0.2f, 1.0f };
	settings.startSize = 1.0f;
	settings.endSize = 0.1f;
	settings.lifeTime = 2.0f;
	settings.position = { 1.0f, 0.0f, 1.0f };
	settings.velocity = { 0.0f, 5.0f, 0.0f };
	settings.velocityVariation = { 2.0f, 1.0f, 2.0f };
	settings.gravity = -3.0f;

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
