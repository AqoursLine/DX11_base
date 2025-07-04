#pragma once

#include "gameObject.h"

class Camera : public GameObject {
public:
	Camera() = default;
	~Camera() = default;

	virtual bool Initialize() override;
	virtual void Finalize() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw() const override;
	virtual void CleanUp() override;

protected:
	//左右移動関数
	void MoveSide(bool isRight, double deltaTime);

	//前後移動関数
	void MoveForward(bool isForward, double deltaTime);

	//移動量
	float m_moveSpeed = 5.0f;
	//回転量
	float m_rotateSpeed = 0.5f;

	Vector3 m_up { 0.0f, 1.0f, 0.0f };
	Vector3 m_right { 1.0f, 0.0f, 0.0f };
	Vector3 m_forward { 0.0f, 0.0f, 1.0f };

	Vector3 m_targetPosition { 0.0f, 0.0f, 0.0f };
	Vector3 m_offset { 0.0f, 3.0f, -10.0f }; // カメラのオフセット位置
};
