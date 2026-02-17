#pragma once

#include "gameObject.h"

class ResultRankBackground : public GameObject {
public:
	ResultRankBackground() = default;
	~ResultRankBackground() = default;

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

	int m_mainPlayerIndex = -1; // メインプレイヤーのインデックス
	int m_playerCount = 0; // プレイヤーの総数

	Vector4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

