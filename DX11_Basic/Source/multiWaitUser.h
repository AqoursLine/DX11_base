#pragma once

#include "gameObject.h"

class Texture;

class MultiWaitUser : public GameObject {
public:
	MultiWaitUser() = default;
	~MultiWaitUser() = default;

	void SetColor(const Vector4& color) { m_color = color; }

	MultiWaitUser* SetIconVisible(bool visible) { m_isIconVisible = visible; return this; }
	bool IsIconVisible() const { return m_isIconVisible; }

	MultiWaitUser* SetReady(bool isReady) { m_isReady = isReady; return this; }
	bool IsReady() const { return m_isReady; }
protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	Texture* m_multiIconBackgroundTexture = nullptr;
	Texture* m_multiIconWaitingTexture = nullptr;
	Texture* m_readyTexture = nullptr;
	Texture* m_notReadyTexture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	Vector4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };

	bool m_isIconVisible = true;

	bool m_isReady = false;

	float m_waitAnimationTime = 0.0f;
};
