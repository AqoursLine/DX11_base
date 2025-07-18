#pragma once

#include "gameObject.h"

class Player : public GameObject {
public:
	Player() = default;
	~Player() = default;

	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;

private:
	class Model* m_model = nullptr;

	float m_moveSpeed = 0.0f; // 移動速度
	float m_acceleration = 0.0f; // 加速度
	float m_deceleration = 0.0f; // 減速度
	float m_maxSpeed = 00.0f; // 最大速度

	float m_sideForce = 0.0f; // 横移動の力
	float m_sideAcceleration = 0.0f; // 横移動の加速度
	float m_sideMaxForce = 0.0f; // 横移動の最大速度
};
