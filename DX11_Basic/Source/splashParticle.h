#pragma once

#include "gpuParticleSystem.h"

class SplashParticle : public GPUParticleSystem {
public:
	SplashParticle() : GPUParticleSystem(ParticleDrawMode::CULL_DISTANCE) {}
	~SplashParticle() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:
	class Texture* m_texture = nullptr;

};
