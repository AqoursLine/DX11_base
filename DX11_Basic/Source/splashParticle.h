#pragma once

#include "particleSystem.h"

class SplashParticle : public ParticleSystem {
public:
	SplashParticle() = default;
	~SplashParticle() = default;

protected:
	bool Initialize() override;
	void Finalize() override;

private:
	class Texture* m_texture = nullptr;

};
