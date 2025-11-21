#pragma once

#include "particleSystem.h"

class SplashParticle : public ParticleSystem {
public:
	SplashParticle() = default;
	~SplashParticle() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;

private:
	class Texture* m_texture = nullptr;

};
