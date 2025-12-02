#include "testParticle.h"
#include "texture.h"
#include "input.h"

bool TestParticle::Initialize() {
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
	settings.endColor = { 1.0f, 1.0f, 0.2f, 0.1f };
	settings.startSize = 1.0f;
	settings.endSize = 0.1f;
	settings.lifeTime = 2.0f;
	settings.position = { 5.0f, 0.0f, 5.0f };
	settings.velocity = { 0.0f, 5.0f, 0.0f };
	settings.velocityVariation = { 2.0f, 1.0f, 2.0f };
	settings.gravity = -3.0f;
	settings.oneShot = true;
	settings.oneShotCount = 50;
	settings.maxParticles = 10000;

	SetEmitterSettings(settings);

	GPUParticleSystem::Initialize();
	return true;
}

void TestParticle::Finalize() {
	if (m_texture) {
		delete m_texture;
		m_texture = nullptr;
	}

	GPUParticleSystem::Finalize();
}

void TestParticle::Update(double deltaTime) {
	if (Input::GetKeyTrigger(KK_D0)) {

		EmitOneShot({ 5.0f, 0.0f, 5.0f });
	}

	if (Input::GetKeyPress(KK_D1)) {
		EmitOneShot({ 0.0f, 0.0f, 0.0f });
	}

	if (Input::GetKeyPress(KK_D2)) {
		EmitOneShot({ 5.0f, 0.0f, 5.0f });
	}

	GPUParticleSystem::Update(deltaTime);
}
