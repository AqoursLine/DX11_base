#pragma once

#include "gameObject.h"

constexpr int MAX_RANKS = 6;

class Texture;

class RankDisplay : public GameObject {
public:
	RankDisplay() = default;
	~RankDisplay() = default;

protected:
	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() override;

private:
	Texture* m_rankTextures[MAX_RANKS] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	class Sprite* m_sprite = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;
	class PixelShader* m_backPixelShader = nullptr;

	class Player* m_player = nullptr;
	float m_alpha = 1.0f;
	float m_fadeStartProgress = 0.9f;
	float m_fadeDuration = 1.0f;
};
