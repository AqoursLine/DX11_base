#pragma once

#include "gameObject.h"

class ResultRankIcon : public GameObject {
public:
	ResultRankIcon() = default;
	~ResultRankIcon() = default;

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

	std::vector<Vector4> m_colors; // 各レーンの色を格納するベクター
};
