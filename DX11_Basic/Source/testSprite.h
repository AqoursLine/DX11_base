#pragma once
#include "gameObject.h"

class TestSprite : public GameObject {
public:
	TestSprite() = default;
	~TestSprite() = default;

	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;
	void CleanUp() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	// Shader
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;
};
