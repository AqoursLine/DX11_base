#pragma once

#include "gpuParticleSystem.h"

class TestParticle : public GPUParticleSystem {
public:
	TestParticle() : GPUParticleSystem(ParticleDrawMode::INDIRECT_DRAW) {}
	~TestParticle() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;

private:
	class Texture* m_texture = nullptr;

	float m_emitTimer = 0.0f;

};
