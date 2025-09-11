#pragma once

#include "vehicle.h"

struct VehicleInput {
	float forward = 0.0f; //アクセル入力	(0.0f ~ 1.0f)
	float reverse = 0.0f; //リバース入力	(0.0f ~ 1.0f)
	float brake = 0.0f; //ブレーキ入力		(0.0f ~ 1.0f)
	float steering = 0.0f; //ハンドル入力	(-1.0f ~ 1.0f)
	bool handbrake = false; //サイドブレーキ	(true/false)
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

	class Field* m_field = nullptr;

	VehicleInput m_currentInput;
	VehicleInput m_smoothedInput;

	//入力平滑化用パラメータ
	float m_forwardSmoothRate = 5.0f; //全身平滑化レート
	float m_reverseSmoothRate = 4.0f; //リバース平滑化レート
	float m_brakeSmoothRate = 8.0f; //ブレーキ平滑化レート
	float m_steerSmoothRate = 6.0f; //ステアリング平滑化レート

	//入力更新
	void UpdateInput(double deltaTime);
	//入力平滑化
	void SmoothInput(double deltaTime);

	//タイヤ描画
	void DrawWheels() const;

};
