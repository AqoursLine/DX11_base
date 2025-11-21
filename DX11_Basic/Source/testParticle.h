#pragma once

#include "particleSystem.h"

class TestParticle : public ParticleSystem {
public:
	TestParticle() = default;
	~TestParticle() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;

private:
	class Texture* m_texture = nullptr;

};
