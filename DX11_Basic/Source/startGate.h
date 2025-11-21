#pragma once

#include "gameObject.h"

class StartGate : public GameObject {
public:
	StartGate() = default;
	~StartGate() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	float m_animationTime = 0.0f;
	float m_animationSpeed = 1.0f;

	class RaceManager* m_raceManager = nullptr;

	bool m_isPassed = false;
	float m_passCheckTime = 0.0f;
};
