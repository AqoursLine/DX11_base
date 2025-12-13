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

	EmitterSettings settings = {};
	settings.startColor = { 1.0f, 0.2f, 0.2f, 1.0f };
	settings.endColor = { 1.0f, 1.0f, 0.2f, 0.1f };
	settings.startSize = 0.2f;
	settings.endSize = 0.01f;
	settings.lifeTime = 5.0f;
	settings.position = { 3.0f, 3.0f, 3.0f };
	settings.velocity = { 0.0f, 0.0f, 10.0f };
	settings.velocityVariation = { 1.0f, 1.0f, 1.0f };
	settings.worldAcceleration = {0.0f, 0.0f, 1.0f};
	settings.localAcceleration = { 10.0f, 10.0f, 0.0f };
	settings.oneShot = true;
	settings.oneShotCount = 800;
	settings.maxParticles = 1000000;

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
	if (Input::GetKeyTrigger(KK_D1)) {

		EmitOneShot({ 0.0f, 0.0f, 0.0f });
	}

	if (Input::GetKeyPress(KK_D2)) {
		EmitOneShot({ 0.0f, 0.0f, 0.0f });
	}

	if (Input::GetKeyTrigger(KK_D3)) {
		m_emitTimer += static_cast<float>(deltaTime);

		if (m_emitTimer <= 0.5f) {

		} else if (m_emitTimer <= 1.0f) {
			EmitOneShot({ 0.0f, 0.0f, 0.0f });
		}
	}

	GPUParticleSystem::Update(deltaTime);
}
