#include "splashParticle.h"
#include "texture.h"
#include "input.h"

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
	settings.startSize = 0.4f;
	settings.endSize = 0.1f;
	settings.lifeTime = 1.2f;
	settings.position = { 0.0f, 0.0f, 0.0f };
	settings.velocity = { 0.0f, 4.0f, 0.0f };
	settings.velocityVariation = { 2.5f, 1.0f, 2.5f };
	settings.gravity = -9.8f;
	settings.maxParticles = 200;
	settings.oneShot = true;
	settings.oneShotCount = 40;

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
