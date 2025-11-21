#pragma once

#include "gameObject.h"

class SpeedMeter : public GameObject {
public:
	SpeedMeter() = default;
	~SpeedMeter() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	class Texture* m_numberTexture = nullptr;
	class Texture* m_needleTexture = nullptr;
	class VertexShader* m_animationVertexShader = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_animationPixelShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;
	class Player* m_player = nullptr;

	float m_speed = 0.0f; //現在の速度(km/h)
};

