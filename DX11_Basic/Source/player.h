#pragma once

#include "racingBoat.h"

struct VehicleInput {
	float throttle = 0.0f; //推進入力		(-1.0f ~ 1.0f)
	float steering = 0.0f; //ハンドル入力	(-1.0f ~ 1.0f)
	float brake = 0.0f;    //ブレーキ入力	(0.0f ~ 1.0f)
};

class Player : public RacingBoat {
public:
	Player() = default;
	~Player() = default;

protected:
	bool Initialize() override;
	void Update(double deltaTime) override;

private:
	VehicleInput m_currentInput;
	VehicleInput m_smoothedInput;

	//入力平滑化用パラメータ
	float m_throttlSmoothRate = 5.0f; //推進平滑化レート
	float m_steerSmoothRate = 6.0f; //ステアリング平滑化レート

	//入力更新
	void UpdateInput(double deltaTime);
	//入力平滑化
	void SmoothInput(double deltaTime);
};
