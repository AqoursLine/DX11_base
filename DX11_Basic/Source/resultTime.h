#pragma once

#include "gameObject.h"

class ResultTime : public GameObject {
public:
	ResultTime() = default;
	~ResultTime() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	class Texture* m_numberTexture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	float m_resultTime = 0.0f;
};
