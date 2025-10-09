#pragma once


#include "gameObject.h"

class RaceTimer : public GameObject {
public:
	RaceTimer() = default;
	~RaceTimer() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;

private:
	class RaceManager* m_raceManager = nullptr;

	class Sprite* m_sprite = nullptr;
	class Texture* m_numberTexture = nullptr;
	class VertexShader* m_animationVertexShader = nullptr;
	class PixelShader* m_animationPixelShader = nullptr;

	float m_raceTime = 0.0f; //レースタイム(秒)
};
