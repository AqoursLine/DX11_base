#pragma once

#include "gameObject.h"

class RaceCountDownText : public GameObject {
public:
	RaceCountDownText() = default;
	~RaceCountDownText() = default;

	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	class RaceManager* m_raceManager = nullptr;

	float m_displayTime = 1.0f; //表示時間
	float m_time = 0.0f; //経過時間
};
