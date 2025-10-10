#pragma once

#include "gameObject.h"

class GoalGate : public GameObject {
public:
	GoalGate() = default;
	~GoalGate() = default;

protected:
	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;

private:
	class Sprite* m_sprite = nullptr;
	class Texture* m_texture = nullptr;
	class VertexShader* m_vertexShader = nullptr;
	class PixelShader* m_pixelShader = nullptr;

	class Texture* m_lapGateTexture = nullptr;
	class Texture* m_lapNumberTexture = nullptr;
	class PixelShader* m_lapGatePS = nullptr;

	float m_animationTime = 0.0f;
	float m_animationSpeed = 1.0f;

	class RaceManager* m_raceManager = nullptr;

	bool m_isPassed = false;	//ゴール通過フラグ

	int m_topLapCount = 0;	//トップの周回数
	int m_lastLapCount = 0; //最下位の周回数
};
