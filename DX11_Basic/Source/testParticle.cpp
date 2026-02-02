#include "testParticle.h"
#include "texture.h"
#include "input.h"

#include "imguiSystem.h"


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
	settings.startSize = 0.01f;
	settings.endSize = 0.001f;
	settings.lifeTime = 2.0f;
	settings.position = { 0.0f, 0.0f, 0.0f };
	settings.velocity = { 0.0f, 0.0f, 0.0f };
	settings.velocityVariation = { 2.0f, 2.0f, 0.0f };
	settings.worldAcceleration = { 0.0f, 0.0f, 10.0f};
	settings.localAcceleration = { 100.0f, 0.0f, 10.0f };
	settings.upVector = { 0.0f, 0.0f, -1.0f };
	settings.oneShot = true;
	settings.oneShotCount = 10000;
	settings.maxParticles = 10000000;

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

void TestParticle::Draw() {

	// ImGuiでパーティクル設定を変更
	ImGui::Begin("Particle Settings");
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


	GPUParticleSystem::Draw();
}
