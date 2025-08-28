#pragma once

#include "vehicle.h"

struct VehicleInput {
	float forward = 0.0f; //アクセル入力
	float reverse = 0.0f; //リバース入力
	float brake = 0.0f; //ブレーキ入力
	float steering = 0.0f; //ハンドル入力
	bool handbrake = false; //サイドブレーキ
};

class Player : public Vehicle {
public:
	Player() = default;
	~Player() = default;

	bool Initialize() override;
	void Finalize() override;
	void Update(double deltaTime) override;
	void Draw() const override;

private:
	class Model* m_model = nullptr;

	class Box* m_box = nullptr;

	VehicleInput m_currentInput;
	VehicleInput m_smoothInput;

	//入力平滑化用パラメータ
	float m_accelSmoothRate = 4.0f; //アクセル平滑化速度
	float m_brakeSmoothRate = 8.0f; //ブレーキ平滑化速度
	float m_steerSmoothRate = 6.0f; //ハンドル平滑化速度

	//入力更新
	void UpdateInput(double deltaTime);
	//入力平滑化
	void SmoothInput(double deltaTime);
};
