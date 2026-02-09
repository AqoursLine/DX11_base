#pragma once

#include "gameObject.h"

class LapDisplay : public GameObject {
public:
	LapDisplay() = default;
	~LapDisplay() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class VertexShader* m_backVertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;
	class PixelShader* m_backPixelShader = nullptr;

	int m_lapCount = 0;
	int m_maxLapCount = 3;

	class RaceManager* m_raceManager = nullptr;
	class Player* m_player = nullptr;

	Vector3 m_numberScale = { 0.0f, 0.0f, 0.0f };
};

