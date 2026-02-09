#pragma once

#include "gameObject.h"

class ResultRankText : public GameObject {
public:
	ResultRankText() = default;
	~ResultRankText() = default;

	void SetRankIndex(int index) { m_rankIndex = index; }

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	// Shader
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	int m_rankIndex = 0; // 0:1位, 1:2位, 2:3位 ...

};
