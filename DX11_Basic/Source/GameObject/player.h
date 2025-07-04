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

	float m_moveSpeed = 30.0f; // 移動速度
	float m_rotateSpeed = 2.0f; // 回転速度
	Vector3 m_velocity { 0.0f, 0.0f, 0.0f }; // 速度ベクトル
	Vector3 m_acceleration { 0.0f, 0.0f, 0.0f }; // 加速度ベクトル
};
