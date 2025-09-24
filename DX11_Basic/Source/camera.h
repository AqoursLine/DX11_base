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

	//上下移動関数
	void MoveUpDown(bool isUp, double deltaTime);

	//ターゲットを中心にしてm_rotationからカメラを回転させる関数
	void RotateAroundTarget(double deltaTime);
	void RotateAroundTargetQuaternion(double deltaTime);

	//移動量
	float m_moveSpeed = 0.0f;
	//回転量
	float m_rotateSpeed = 0.0f;

	Vector3 m_targetPosition { 0.0f, 0.0f, 0.0f };
	Vector3 m_offset { 0.0f, 0.0f, 0.0f }; // カメラのオフセット位置
	Vector3 m_rotationOffset { 0.0f, 0.0f, 0.0f };

	//レート
	float m_rate = 0.0f;

	//オフセット減衰スタートタイマー
	float m_offsetDampingTimer = 0.0f;

private:
};
