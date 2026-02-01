#pragma once

#include "gameObject.h"

class ReadyButton : public GameObject {
public:
	ReadyButton() = default;
	~ReadyButton() = default;

	void SetReady(bool isReady) { m_isReady = isReady; }

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

	bool m_isReady = false;
};

