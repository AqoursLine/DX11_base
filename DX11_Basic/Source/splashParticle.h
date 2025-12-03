#pragma once

#include "gpuParticleSystem.h"

class SplashParticle : public GPUParticleSystem {
public:
	SplashParticle() : GPUParticleSystem(ParticleDrawMode::INDIRECT_DRAW) {}
//	SplashParticle() = default;
	~SplashParticle() = default;

protected:
	bool Initialize() override;
	void Finalize() override;


private:
	class Texture* m_texture = nullptr;

};
