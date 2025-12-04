#pragma once

#include "transition.h"

class Texture;

class TestTransition : public Transition {
public:
	TestTransition() = default;
	~TestTransition() = default;

	bool Initialize() override;
	void Finalize() override;
	void Draw() override;

protected:
	void UpdateInTransition(double deltaTime) override;
	void UpdateTransition(double deltaTime) override;
	void UpdateOutTransition(double deltaTime) override;

private:
	Texture* m_fadeTexture = nullptr;
	Texture* m_logoTexture = nullptr;

	class Sprite* m_spriteRenderer = nullptr;
	class PixelShader* m_pixelShader = nullptr;
	class VertexShader* m_vertexShader = nullptr;

	float m_inTimer = 0.0f;
	float m_inDuration = 1.5f;
	float m_outTimer = 0.0f;
	float m_outDuration = 1.5f;

	float m_alpha = 0.0f;

	float m_logoRotate = 0.0f;
	float m_logoRotateSpeed = 60.0f; // 1秒あたりの回転角度
};
