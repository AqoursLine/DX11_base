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
	void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_numberTexture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	std::vector<float> m_times; // 各レーンのタイムを格納するベクター
	int m_mainPlayerIndex = -1; // メインプレイヤーのインデックス
};
